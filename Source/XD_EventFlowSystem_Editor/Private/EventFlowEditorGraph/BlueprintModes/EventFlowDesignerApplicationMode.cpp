// Fill out your copyright notice in the Description page of Project Settings.

#include "EventFlowDesignerApplicationMode.h"
#include "WorkflowTabFactory.h"
#include "EventFlowSystemEditor.h"
#include "BlueprintEditor.h"
#include "PropertyEditorModule.h"
#include "GraphEditor.h"
#include "GenericCommands.h"
#include "EdGraphUtilities.h"
#include "PlatformApplicationMisc.h"
#include "SBlueprintEditorToolbar.h"
#include "BlueprintEditorUtils.h"
#include "EventFlowSystemEditorNode.h"
#include "EventFlowSystemEditorGraph.h"
#include "PropertyEditorDelegates.h"
#include "SKismetInspector.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "SComboButton.h"
#include "DetailWidgetRow.h"
#include "ObjectEditorUtils.h"
#include "KismetEditorUtilities.h"
#include "SWidgetSwitcher.h"
#include "K2Node_ComponentBoundEvent.h"
#include "IDetailPropertyExtensionHandler.h"
#include "MultiBoxBuilder.h"
#include "SlateApplication.h"
#include "ScopedTransaction.h"
#include "EventFlowGraphBlueprint.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowGraph"

const FName FEventFlowDesignerApplicationMode::DetailsTabId(TEXT("EventFlowDesigner_DetailsTabId"));
const FName FEventFlowDesignerApplicationMode::GraphTabId(TEXT("EventFlowDesigner_GraphTabId"));

struct FFunctionInfo
{
	FFunctionInfo()
		: Function(nullptr)
	{
	}

	FText DisplayName;
	FString Tooltip;

	FName FuncName;
	UFunction* Function;
};

class SEventFlowDetailsView : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SEventFlowDetailsView) {}
	SLATE_END_ARGS()
public:
	TSharedPtr<SEditableTextBox> NameTextBox;
	TSharedPtr<class IDetailsView> PropertyView;

	TWeakPtr<FEventFlowSystemEditor> Editor;

	void Construct(const FArguments& InArgs, TWeakPtr<FEventFlowSystemEditor> InEditor)
	{
		Editor = InEditor;

		FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FNotifyHook* NotifyHook = this;

		FDetailsViewArgs DetailsViewArgs(
			/*bUpdateFromSelection=*/ false,
			/*bLockable=*/ false,
			/*bAllowSearch=*/ true,
			FDetailsViewArgs::HideNameArea,
			/*bHideSelectionTip=*/ true,
			/*InNotifyHook=*/ NotifyHook,
			/*InSearchInitialKeyFocus=*/ false,
			/*InViewIdentifier=*/ NAME_None);
		DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;

		PropertyView = EditModule.CreateDetailView(DetailsViewArgs);

		class SPropertyBinding : public SCompoundWidget
		{
		public:

			SLATE_BEGIN_ARGS(SPropertyBinding)
			{}
			SLATE_END_ARGS()

		public:
			void Construct(const FArguments& InArgs, FEventFlowSystemEditor* InEditor, UDelegateProperty* DelegateProperty, TSharedRef<IPropertyHandle> Property)
			{
				Editor = InEditor;
				BindableSignature = DelegateProperty->SignatureFunction;

				TArray<UObject*> Objects;
				Property->GetOuterObjects(Objects);
				UObject* Object = Objects[0];

				ChildSlot
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SComboButton)
						.OnGetMenuContent(this, &SPropertyBinding::OnGenerateDelegateMenu, Object, Property)
						.ContentPadding(1)
						.ButtonContent()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(this, &SPropertyBinding::GetCurrentBindingImage, Property)
								.ColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.25f))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4, 1, 0, 0)
							[
								SNew(STextBlock)
								.Text(this, &SPropertyBinding::GetCurrentBindingText, Property)
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
						.Visibility(this, &SPropertyBinding::GetGotoBindingVisibility, Property)
						.OnClicked(this, &SPropertyBinding::HandleGotoBindingClicked, Property)
						.VAlign(VAlign_Center)
						.ToolTipText(LOCTEXT("GotoFunction", "Goto Function"))
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Browse"))
						]
					]
				];
			}

			TSharedRef<SWidget> OnGenerateDelegateMenu(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle)
			{
				const bool bInShouldCloseWindowAfterMenuSelection = true;
				FMenuBuilder MenuBuilder(bInShouldCloseWindowAfterMenuSelection, nullptr);

				MenuBuilder.BeginSection("BindingActions");
				{
					if (CanRemoveBinding(PropertyHandle))
					{
						MenuBuilder.AddMenuEntry(
							LOCTEXT("RemoveBinding", "移除绑定"),
							LOCTEXT("RemoveBindingToolTip", "移除当前绑定"),
							FSlateIcon(FEditorStyle::GetStyleSetName(), "Cross"),
							FUIAction(FExecuteAction::CreateSP(this, &SPropertyBinding::HandleRemoveBinding, PropertyHandle))
						);
					}
					else
					{
						MenuBuilder.AddMenuEntry(
							LOCTEXT("CreateBinding", "添加绑定"),
							LOCTEXT("CreateBindingToolTip", "在当前的属性上创建函数绑定"),
							FSlateIcon(FEditorStyle::GetStyleSetName(), "Plus"),
							FUIAction(FExecuteAction::CreateSP(this, &SPropertyBinding::HandleCreateAndAddBinding, Object, PropertyHandle))
						);
					}
				}
				MenuBuilder.EndSection(); //CreateBinding

				FDisplayMetrics DisplayMetrics;
				FSlateApplication::Get().GetCachedDisplayMetrics(DisplayMetrics);

				return
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.MaxHeight(DisplayMetrics.PrimaryDisplayHeight * 0.5)
					[
						MenuBuilder.MakeWidget()
					];
			}

			const FSlateBrush* GetCurrentBindingImage(TSharedRef<IPropertyHandle> PropertyHandle) const
			{
				static FName PropertyIcon(TEXT("Kismet.Tabs.Variables"));
				static FName FunctionIcon(TEXT("GraphEditor.Function_16x"));

				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);

				//TODO UMG O(N) Isn't good for this, needs to be map, but map isn't serialized, need cached runtime map for fast lookups.

				FName PropertyName = PropertyHandle->GetProperty()->GetFName();
				for (int32 ObjectIndex = 0; ObjectIndex < OuterObjects.Num(); ObjectIndex++)
				{
					// Ignore null outer objects
					if (OuterObjects[ObjectIndex] == NULL)
					{
						continue;
					}

					if (!CanRemoveBinding(PropertyHandle))
					{
						return FEditorStyle::GetBrush(FunctionIcon);
					}
				}

				return nullptr;
			}

			FText GetCurrentBindingText(TSharedRef<IPropertyHandle> PropertyHandle) const
			{
				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);

				FName PropertyName = PropertyHandle->GetProperty()->GetFName();
				for (int32 ObjectIndex = 0; ObjectIndex < OuterObjects.Num(); ObjectIndex++)
				{
					// Ignore null outer objects
					if (OuterObjects[ObjectIndex] == nullptr)
					{
						continue;
					}

					UEventFlowGraphBlueprint* Blueprint = Editor->GetEventFlowBlueprint();

					for (const FEventFlowDelegateEditorBinding& Binding : Blueprint->Bindings)
					{
						if (Binding.Object.Get() == OuterObjects[ObjectIndex] && Binding.PropertyName == PropertyName)
						{
							TArray<UEdGraph*> AllGraphs;
							Blueprint->GetAllGraphs(AllGraphs);

							FGuid SearchForGuid = Binding.MemberFunctionGuid;

							for (UEdGraph* Graph : AllGraphs)
							{
								if (Graph->GraphGuid == SearchForGuid)
								{
									FName FoundName = Blueprint->GetFieldNameFromClassByGuid<UFunction>(Blueprint->GeneratedClass, Binding.MemberFunctionGuid);
									return FText::FromString(FName::NameToDisplayString(FoundName.ToString(), false));
								}
							}

							return LOCTEXT("绑定函数丢失", "绑定函数丢失");
						}
					}

					break;
				}

				return LOCTEXT("未绑定", "未绑定");
			}

			EVisibility GetGotoBindingVisibility(TSharedRef<IPropertyHandle> PropertyHandle) const
			{
				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);

				FName PropertyName = PropertyHandle->GetProperty()->GetFName();
				for (int32 ObjectIndex = 0; ObjectIndex < OuterObjects.Num(); ObjectIndex++)
				{
					// Ignore null outer objects
					if (OuterObjects[ObjectIndex] == nullptr)
					{
						continue;
					}

					UEventFlowGraphBlueprint* Blueprint = Editor->GetEventFlowBlueprint();

					for (const FEventFlowDelegateEditorBinding& Binding : Blueprint->Bindings)
					{
						if (Binding.Object.Get() == OuterObjects[ObjectIndex])
						{
							TArray<UEdGraph*> AllGraphs;
							Blueprint->GetAllGraphs(AllGraphs);

							FGuid SearchForGuid = Binding.MemberFunctionGuid;

							for (UEdGraph* Graph : AllGraphs)
							{
								if (Graph->GraphGuid == SearchForGuid)
								{
									return EVisibility::Visible;
								}
							}
						}
					}

				}

				return EVisibility::Collapsed;
			}

			FReply HandleGotoBindingClicked(TSharedRef<IPropertyHandle> PropertyHandle)
			{
				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);

				FName PropertyName = PropertyHandle->GetProperty()->GetFName();
				for (int32 ObjectIndex = 0; ObjectIndex < OuterObjects.Num(); ObjectIndex++)
				{
					// Ignore null outer objects
					if (OuterObjects[ObjectIndex] == nullptr)
					{
						continue;
					}

					UEventFlowGraphBlueprint* Blueprint = Editor->GetEventFlowBlueprint();

 					for (const FEventFlowDelegateEditorBinding& Binding : Blueprint->Bindings)
 					{
						if (Binding.Object.Get() == OuterObjects[ObjectIndex] && Binding.PropertyName == PropertyName)
						{
							TArray<UEdGraph*> AllGraphs;
							Blueprint->GetAllGraphs(AllGraphs);

							FGuid SearchForGuid = Binding.MemberFunctionGuid;

							for (UEdGraph* Graph : AllGraphs)
							{
								if (Graph->GraphGuid == SearchForGuid)
								{
									GotoFunction(Graph);
									return FReply::Handled();
								}
							}
						}
 					}
				}

				return FReply::Unhandled();
			}

			bool CanRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle) const
			{
				FName PropertyName = PropertyHandle->GetProperty()->GetFName();

				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);
				for (UObject* SelectedObject : OuterObjects)
				{
					UEventFlowGraphBlueprint* Blueprint = Editor->GetEventFlowBlueprint();

					for (const FEventFlowDelegateEditorBinding& Binding : Blueprint->Bindings)
					{
						if (Binding.Object.Get() == SelectedObject && Binding.PropertyName == PropertyName)
						{
							return true;
						}
					}
				}

				return false;
			}

			void HandleRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle)
			{
				UEventFlowGraphBlueprint* Blueprint = Editor->GetEventFlowBlueprint();

				const FScopedTransaction Transaction(LOCTEXT("UnbindDelegate", "Remove Binding"));

				Blueprint->Modify();

				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);
				for (UObject* SelectedObject : OuterObjects)
				{
					Blueprint->Bindings.RemoveAll([&](const FEventFlowDelegateEditorBinding& Binding) {return Binding.Object == SelectedObject && Binding.PropertyName == PropertyHandle->GetProperty()->GetFName(); });
				}			

				FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
			}

			void HandleCreateAndAddBinding(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle)
			{
				UEventFlowGraphBlueprint* Blueprint = Editor->GetEventFlowBlueprint();

				const FScopedTransaction Transaction(LOCTEXT("CreateDelegate", "Create Binding"));

				Blueprint->Modify();

				FString Pre = GeneratePureBindings ? FString(TEXT("Get")) : FString(TEXT("On"));
 				FString	ObjectName = TEXT("_") + Object->GetName() + TEXT("_");

				FString Post = PropertyHandle->GetProperty()->GetName();
				Post.RemoveFromStart(TEXT("On"));
				Post.RemoveFromEnd(TEXT("Event"));

				// Create the function graph.
				FString FunctionName = Pre + ObjectName + Post;
				UEdGraph* FunctionGraph = FBlueprintEditorUtils::CreateNewGraph(
					Blueprint,
					FBlueprintEditorUtils::FindUniqueKismetName(Blueprint, FunctionName),
					UEdGraph::StaticClass(),
					UEdGraphSchema_K2::StaticClass());

				// Add the binding to the blueprint
				TSharedPtr<FFunctionInfo> SelectedFunction = MakeShareable(new FFunctionInfo());
				SelectedFunction->FuncName = FunctionGraph->GetFName();

				const bool bUserCreated = true;
				FBlueprintEditorUtils::AddFunctionGraph(Blueprint, FunctionGraph, bUserCreated, BindableSignature);

				// Only mark bindings as pure that need to be.
				if (GeneratePureBindings)
				{
					const UEdGraphSchema_K2* Schema_K2 = Cast<UEdGraphSchema_K2>(FunctionGraph->GetSchema());
					Schema_K2->AddExtraFunctionFlags(FunctionGraph, FUNC_BlueprintPure);
				}

				FEventFlowDelegateEditorBinding Binding;
				Binding.Object = Object;
				Binding.PropertyName = PropertyHandle->GetProperty()->GetFName();
				Binding.MemberFunctionGuid = FunctionGraph->GraphGuid;
				Blueprint->Bindings.Add(Binding);

				GotoFunction(FunctionGraph);
			}

			void GotoFunction(UEdGraph* FunctionGraph)
			{
				Editor->SetCurrentMode(FBlueprintApplicationModesTemplate::GraphMode);

				Editor->OpenDocument(FunctionGraph, FDocumentTracker::OpenNewDocument);
			}

		private:
			FEventFlowSystemEditor* Editor;

			bool GeneratePureBindings = true;
			UFunction* BindableSignature;
		};

		class FEventFlowDetailExtensionHandler : public IDetailPropertyExtensionHandler
		{
		public:
			FEventFlowDetailExtensionHandler(FEventFlowSystemEditor* BlueprintEditor)
				: BlueprintEditor(BlueprintEditor)
			{}

			virtual bool IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& InPropertyHandle) const override
			{
				if (InPropertyHandle.GetNumOuterObjects() == 1)
				{
					TArray<UObject*> Objects;
					InPropertyHandle.GetOuterObjects(Objects);

					// We don't allow bindings on the CDO.
					if (Objects[0] != nullptr && Objects[0]->HasAnyFlags(RF_ClassDefaultObject))
					{
						return false;
					}

					UProperty* Property = InPropertyHandle.GetProperty();
					FString DelegateName = Property->GetName() + "Delegate";

					if (UClass* ContainerClass = Cast<UClass>(Property->GetOuter()))
					{
						UDelegateProperty* DelegateProperty = FindField<UDelegateProperty>(ContainerClass, FName(*DelegateName));
						if (DelegateProperty)
						{
							return true;
						}
					}
				}

				return false;
			}

			virtual TSharedRef<SWidget> GenerateExtensionWidget(const UClass* InObjectClass, TSharedPtr<IPropertyHandle> InPropertyHandle) override
			{
				UProperty* Property = InPropertyHandle->GetProperty();
				FString DelegateName = Property->GetName() + "Delegate";

				UDelegateProperty* DelegateProperty = FindFieldChecked<UDelegateProperty>(CastChecked<UClass>(Property->GetOuter()), FName(*DelegateName));

				const bool bIsEditable = Property->HasAnyPropertyFlags(CPF_Edit | CPF_EditConst);
				const bool bDoSignaturesMatch = DelegateProperty->SignatureFunction->GetReturnProperty()->SameType(Property);

				if (!ensure(bIsEditable && bDoSignaturesMatch))
				{
					return SNullWidget::NullWidget;
				}

				return SNew(SPropertyBinding, BlueprintEditor, DelegateProperty, InPropertyHandle.ToSharedRef());
			}

		private:
			FEventFlowSystemEditor* BlueprintEditor;
		};

		PropertyView->SetExtensionHandler(MakeShareable(new FEventFlowDetailExtensionHandler(Editor.Pin().Get())));

		class FEventFlowDesignerDelegate : public IDetailCustomization
		{
			FEventFlowDesignerDelegate(FEventFlowSystemEditor* InEditor, UEventFlowGraphBlueprint* Blueprint)
				:Editor(InEditor), Blueprint(Blueprint)
			{}

			FEventFlowSystemEditor* Editor;
			UEventFlowGraphBlueprint* Blueprint;

			void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
			{
				TArray< TWeakObjectPtr<UObject> > OutObjects;
				DetailLayout.GetObjectsBeingCustomized(OutObjects);

				if (OutObjects.Num() == 1)
				{
					UEventFlowGraphNodeBase* BpNode = Cast<UEventFlowGraphNodeBase>(OutObjects[0].Get());
					UClass* PropertyClass = BpNode->GetClass();

					for (TFieldIterator<UProperty> PropertyIt(PropertyClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
					{
						UProperty* Property = *PropertyIt;
						
						if (UMulticastDelegateProperty* MulticastDelegateProperty = Cast<UMulticastDelegateProperty>(Property))
						{
							CreateMulticastEventCustomization(DetailLayout, *BpNode->GetVarRefName(), PropertyClass, MulticastDelegateProperty);
						}
					}
				}
			}

			void CreateMulticastEventCustomization(IDetailLayoutBuilder& DetailLayout, FName ThisComponentName, UClass* PropertyClass, UMulticastDelegateProperty* DelegateProperty)
			{
				const FString AddString = FString(TEXT("添加 "));
				const FString ViewString = FString(TEXT("查看 "));

				const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

				if ( !K2Schema->CanUserKismetAccessVariable(DelegateProperty, PropertyClass, UEdGraphSchema_K2::MustBeDelegate) )
				{
					return;
				}

				FText PropertyTooltip = DelegateProperty->GetToolTipText();
				if ( PropertyTooltip.IsEmpty() )
				{
					PropertyTooltip = FText::FromString(DelegateProperty->GetName());
				}

				UObjectProperty* ComponentProperty = FindField<UObjectProperty>(Blueprint->SkeletonGeneratedClass, ThisComponentName);

				if (!ComponentProperty)
				{
					return;
				}

				FName PropertyName = ComponentProperty->GetFName();
				FName EventName = DelegateProperty->GetFName();
				FText EventText = DelegateProperty->GetDisplayNameText();

				IDetailCategoryBuilder& EventCategory = DetailLayout.EditCategory(TEXT("事件"), LOCTEXT("事件", "事件"), ECategoryPriority::Uncommon);

				EventCategory.AddCustomRow(EventText)
					.NameContent()
					[
						SNew(SHorizontalBox)
						.ToolTipText(DelegateProperty->GetToolTipText())
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0, 0, 5, 0)
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("GraphEditor.Event_16x"))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.Text(EventText)
						]
					]
					.ValueContent()
					.MinDesiredWidth(150)
					.MaxDesiredWidth(200)
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
						.HAlign(HAlign_Center)
						.OnClicked(this, &FEventFlowDesignerDelegate::HandleAddOrViewEventForVariable, EventName, PropertyName, MakeWeakObjectPtr(PropertyClass))
						.ForegroundColor(FSlateColor::UseForeground())
						[
							SNew(SWidgetSwitcher)
							.WidgetIndex(this, &FEventFlowDesignerDelegate::HandleAddOrViewIndexForButton, EventName, PropertyName)
							+ SWidgetSwitcher::Slot()
							[
								SNew(STextBlock)
								.Font(FEditorStyle::GetFontStyle(TEXT("BoldFont")))
								.Text(LOCTEXT("ViewEvent", "View"))
							]
							+ SWidgetSwitcher::Slot()
							[
								SNew(SImage)
								.Image(FEditorStyle::GetBrush("Plus"))
							]
						]
					];
			}

			FReply HandleAddOrViewEventForVariable(const FName EventName, FName PropertyName, TWeakObjectPtr<UClass> PropertyClass)
			{
				UBlueprint* BlueprintObj = Blueprint;

				// Find the corresponding variable property in the Blueprint
				UObjectProperty* VariableProperty = FindField<UObjectProperty>(BlueprintObj->SkeletonGeneratedClass, PropertyName);

				if (VariableProperty)
				{
					if (!FKismetEditorUtilities::FindBoundEventForComponent(BlueprintObj, EventName, VariableProperty->GetFName()))
					{
						FKismetEditorUtilities::CreateNewBoundEventForClass(PropertyClass.Get(), EventName, BlueprintObj, VariableProperty);
					}
					else
					{
						const UK2Node_ComponentBoundEvent* ExistingNode = FKismetEditorUtilities::FindBoundEventForComponent(BlueprintObj, EventName, VariableProperty->GetFName());
						if (ExistingNode)
						{
							FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(ExistingNode);
						}
					}
				}

				return FReply::Handled();
			}

			int32 HandleAddOrViewIndexForButton(const FName EventName, FName PropertyName) const
			{
				UBlueprint* BlueprintObj = Blueprint;

				if (FKismetEditorUtilities::FindBoundEventForComponent(BlueprintObj, EventName, PropertyName))
				{
					return 0; // View
				}

				return 1; // Add
			}
		public:
			static TSharedRef<IDetailCustomization> MakeInstance(FEventFlowSystemEditor* Editor, UEventFlowGraphBlueprint* Blueprint)
			{
				return MakeShareable(new FEventFlowDesignerDelegate(Editor, Blueprint));
			}
		};

		FEventFlowSystemEditor* BlueprintEditor = Editor.Pin().Get();
		PropertyView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FEventFlowDesignerDelegate::MakeInstance, BlueprintEditor, BlueprintEditor->GetEventFlowBlueprint()));

		ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 6)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SEventFlowDetailsView::GetNameAreaVisibility)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 3, 0)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(this, &SEventFlowDetailsView::GetNameIcon)
				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.Padding(0, 0, 6, 0)
				[
					SAssignNew(NameTextBox, SEditableTextBox)
					.SelectAllTextWhenFocused(true)
					.HintText(LOCTEXT("Name", "Name"))
					.Text(this, &SEventFlowDetailsView::GetNameText)
					.OnTextChanged(this, &SEventFlowDetailsView::HandleNameTextChanged)
					.OnTextCommitted(this, &SEventFlowDetailsView::HandleNameTextCommitted)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked(this, &SEventFlowDetailsView::GetIsVariable)
					.OnCheckStateChanged(this, &SEventFlowDetailsView::HandleIsVariableChanged)
					.Padding(FMargin(3,1,3,1))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("蓝图可见", "蓝图可见"))
					]
				]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				PropertyView.ToSharedRef()
			]
		];
	}

	EVisibility GetNameAreaVisibility() const { return EVisibility::Visible; }

	const FSlateBrush* GetNameIcon() const { return nullptr; }

	FText GetNameText() const
	{
		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		if (Selections.Num() == 1)
		{
			if (UObject* Obj = Selections[0].Get())
			{
				return FText::FromString(Obj->GetName());
			}
		}

		return FText::GetEmpty();
	}

	bool HandleVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
	{
		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		if (Selections.Num() == 1)
		{
			if (UObject* Obj = Selections[0].Get())
			{
				if (Obj->Rename(*InText.ToString(), nullptr, REN_Test))
				{
					return true;
				}
				else
				{
					OutErrorMessage = LOCTEXT("Rename Error", "存在重名对象，无法重命名");
					return false;
				}
			}
		}
		return false;
	}

	void HandleNameTextChanged(const FText& Text)
	{
		FText OutErrorMessage;
		if (!HandleVerifyNameTextChanged(Text, OutErrorMessage))
		{
			NameTextBox->SetError(OutErrorMessage);
		}
		else
		{
			NameTextBox->SetError(FText::GetEmpty());
		}
	}

	void HandleNameTextCommitted(const FText& Text, ETextCommit::Type CommitType)
	{
		UEventFlowGraphBlueprint* Blueprint = Editor.Pin()->GetEventFlowBlueprint();

		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		if (Selections.Num() == 1)
		{
			if (UEventFlowGraphNodeBase* Node = Cast<UEventFlowGraphNodeBase>(Selections[0].Get()))
			{
				const TCHAR* NewName = *Text.ToString();

				if (Node->Rename(NewName, nullptr, REN_Test))
				{
					if (Node->bIsVariable)
					{
						FBlueprintEditorUtils::ReplaceVariableReferences(Blueprint, Node->GetFName(), NewName);
						FBlueprintEditorUtils::ValidateBlueprintChildVariables(Blueprint, NewName);
						FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
					}

					FEventFlowDelegateEditorBinding* Binding = Blueprint->Bindings.FindByPredicate([&](const FEventFlowDelegateEditorBinding& E) {return E.Object.Get() == Node; });

					Node->Rename(NewName);

					if (Binding)
					{
						Binding->Object = Node;
					}

					Editor.Pin()->GetEditorGraph()->RefreshNodes();
				}
			}
		}

		if (CommitType == ETextCommit::OnUserMovedFocus || CommitType == ETextCommit::OnCleared)
		{
			NameTextBox->SetError(FText::GetEmpty());
		}
	}

	ECheckBoxState GetIsVariable() const 
	{
		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		if (Selections.Num() == 1)
		{
			if (UEventFlowGraphNodeBase* Node = Cast<UEventFlowGraphNodeBase>(Selections[0].Get()))
			{
				return Node->bIsVariable ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			}
		}
		return ECheckBoxState::Unchecked; 
	}

	void HandleIsVariableChanged(ECheckBoxState CheckState)
	{
		const FScopedTransaction Transaction(LOCTEXT("VariableToggle", "Variable Toggle"));

		UEventFlowGraphBlueprint* Blueprint = Editor.Pin()->GetEventFlowBlueprint();

		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		for (TWeakObjectPtr<UObject> Obj : Selections)
		{
			if (UEventFlowGraphNodeBase* Node = Cast<UEventFlowGraphNodeBase>(Obj.Get()))
			{
				FName VarName = Node->GetFName();
				Node->bIsVariable = (CheckState == ECheckBoxState::Checked);
			}
		}
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

		PropertyView->ForceRefresh();
	}
};

struct FEventFlowDesignerDetailsSummoner : public FWorkflowTabFactory
{
public:
	FEventFlowDesignerDetailsSummoner(class FEventFlowDesignerApplicationMode* DesignerApplicationMode, TSharedPtr<class FEventFlowSystemEditor> InDesignGraphEditor);

	FEventFlowDesignerApplicationMode* DesignerApplicationMode;
	TWeakPtr<class FEventFlowSystemEditor> InDesignGraphEditor;

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FEventFlowDesignerDetailsSummoner::FEventFlowDesignerDetailsSummoner(class FEventFlowDesignerApplicationMode* DesignerApplicationMode, TSharedPtr<class FEventFlowSystemEditor> InDesignGraphEditor)
	: FWorkflowTabFactory(FEventFlowDesignerApplicationMode::DetailsTabId, InDesignGraphEditor),
	DesignerApplicationMode(DesignerApplicationMode),
	InDesignGraphEditor(InDesignGraphEditor)
{
	TabLabel = LOCTEXT("EventFlowDesingerDetails_TabLabel", "细节");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("EventFlow_DesingerDetails_ViewMenu_Desc", "细节");
	ViewMenuTooltip = LOCTEXT("EventFlow_DesingerDetails_ViewMenu_ToolTip", "Show the Details");
}

TSharedRef<SWidget> FEventFlowDesignerDetailsSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedRef<SEventFlowDetailsView> DesignerDetails = SNew(SEventFlowDetailsView, InDesignGraphEditor);
	DesignerApplicationMode->DesignerDetails = DesignerDetails;
	return DesignerDetails;
}

struct FEventFlowDesignerGraphSummoner : public FWorkflowTabFactory
{
public:
	FEventFlowDesignerGraphSummoner(class FEventFlowDesignerApplicationMode* DesignerApplicationMode, TSharedPtr<class FEventFlowSystemEditor> InDesignGraphEditor);

	FEventFlowDesignerApplicationMode* DesignerApplicationMode;
	TWeakPtr<class FEventFlowSystemEditor> InDesignGraphEditor;

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FEventFlowDesignerGraphSummoner::FEventFlowDesignerGraphSummoner(class FEventFlowDesignerApplicationMode* DesignerApplicationMode, TSharedPtr<class FEventFlowSystemEditor> InDesignGraphEditor)
	:FWorkflowTabFactory(FEventFlowDesignerApplicationMode::GraphTabId, InDesignGraphEditor),
	DesignerApplicationMode(DesignerApplicationMode),
	InDesignGraphEditor(InDesignGraphEditor)
{
	TabLabel = LOCTEXT("EventFlowDesingerGraph_TabLabel", "图表");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "PhysicsAssetEditor.Tabs.Graph");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("EventFlowDesingerGraph_ViewMenu_Desc", "图表");
	ViewMenuTooltip = LOCTEXT("EventFlowDesingerGraph_ViewMenu_ToolTip", "Show the Garph");
}

TSharedRef<SWidget> FEventFlowDesignerGraphSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SAssignNew(DesignerApplicationMode->DesignerGraphEditor, SGraphEditor)
		.AdditionalCommands(DesignerApplicationMode->DesignerEditorCommands)
		.GraphToEdit(InDesignGraphEditor.Pin()->GetEventFlowBlueprint()->EdGraph)
		.GraphEvents(DesignerApplicationMode->DesignerGraphEvents);
}

FEventFlowDesignerApplicationMode::FEventFlowDesignerApplicationMode(TSharedPtr<class FEventFlowSystemEditor> GraphEditorToolkit)
	:FEventFlowBlueprintApplicationModeBase(GraphEditorToolkit, FBlueprintApplicationModesTemplate::DesignerMode)
{
	WorkspaceMenuCategory = FWorkspaceItem::NewGroup(LOCTEXT("EventFlowWorkspaceMenu_Designer", "EventFlowDesigner"));

	TabLayout = FTabManager::NewLayout("EventFlowDesigner_Layout_v1_1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.2f)
				->AddTab(GraphEditorToolkit->GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.8f)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.8f)
					->AddTab(GraphTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.2f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
			)
		);

 	TabFactories.RegisterFactory(MakeShareable(new FEventFlowDesignerDetailsSummoner(this, GraphEditorToolkit)));
 	TabFactories.RegisterFactory(MakeShareable(new FEventFlowDesignerGraphSummoner(this, GraphEditorToolkit)));

	ToolbarExtender = MakeShareable(new FExtender);
	GraphEditorToolkit->GetToolbarBuilder()->AddCompileToolbar(ToolbarExtender);
	GraphEditorToolkit->GetToolbarBuilder()->AddDebuggingToolbar(ToolbarExtender);
	AddModeSwitchToolBarExtension();

	BindDesignerToolkitCommands();
	DesignerGraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateRaw(this, &FEventFlowDesignerApplicationMode::HandleSelectionChanged);
}

void FEventFlowDesignerApplicationMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	FEventFlowSystemEditor *BP = GetBlueprintEditor();

	BP->RegisterToolbarTab(InTabManager.ToSharedRef());
	BP->PushTabFactories(TabFactories);
}

void FEventFlowDesignerApplicationMode::PreDeactivateMode()
{

}

void FEventFlowDesignerApplicationMode::PostActivateMode()
{

}

void FEventFlowDesignerApplicationMode::HandleSelectionChanged(const FGraphPanelSelectionSet& SelectionSet)
{
	if (DesignerDetails.IsValid())
	{
		TSharedPtr<SEventFlowDetailsView> Details = DesignerDetails.Pin();

		TArray<UObject*> ShowObjects;
		for (UObject* Obj : SelectionSet)
		{
			if (UEventFlowSystemEditorNodeBase* EventSystemEdGraphNode = Cast<UEventFlowSystemEditorNodeBase>(Obj))
			{
				if (EventSystemEdGraphNode->EventFlowBpNode)
				{
					ShowObjects.Add(EventSystemEdGraphNode->EventFlowBpNode);
				}
			}
		}

		Details->PropertyView->SetObjects(ShowObjects, true);
	}
}

void FEventFlowDesignerApplicationMode::BindDesignerToolkitCommands()
{
	if (!DesignerEditorCommands.IsValid())
	{
		DesignerEditorCommands = MakeShareable(new FUICommandList());

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandSelectAllNodes),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerSelectAllNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Cut,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandCut),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerCutNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Copy,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandCopy),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerCopyNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Paste,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandPaste),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerPasteNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandDuplicate),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerDuplicateNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandDelete),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerDeleteNodes)
		);

	}
}

FGraphPanelSelectionSet FEventFlowDesignerApplicationMode::GetSelectedNodes()
{
	return GetDesignerGraphEditor()->GetSelectedNodes();
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandSelectAllNodes()
{
	if (DesignerGraphEditor.IsValid())
	{
		GetDesignerGraphEditor()->SelectAllNodes();
	}
}

bool FEventFlowDesignerApplicationMode::CanDesignerSelectAllNodes()
{
	return true;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandCut()
{
	OnDesignerCommandCopy();

	const FGraphPanelSelectionSet OldSelectedNodes = GetDesignerGraphEditor()->GetSelectedNodes();
	GetDesignerGraphEditor()->ClearSelectionSet();
	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
		if (Node && Node->CanDuplicateNode())
		{
			GetDesignerGraphEditor()->SetNodeSelection(Node, true);
		}
	}

	OnDesignerCommandDelete();

	GetDesignerGraphEditor()->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
		if (Node)
			GetDesignerGraphEditor()->SetNodeSelection(Node, true);
	}
}

bool FEventFlowDesignerApplicationMode::CanDesignerCutNodes()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*FGraphPanelSelectionSet::TConstIterator(SelectedNodes)))
		{
			return Node->CanDuplicateNode();
		}
	}
	return true;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandCopy()
{
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	FString ExportedText;

	for (FGraphPanelSelectionSet::TIterator it(SelectedNodes); it; ++it)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*it);
		if (Node)
			Node->PrepareForCopying();
		else
			it.RemoveCurrent();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	for (FGraphPanelSelectionSet::TIterator it(SelectedNodes); it; ++it)
	{
		UEventFlowSystemEditorNodeBase* Node = Cast<UEventFlowSystemEditorNodeBase>(*it);
		if (Node)
			Node->PostCopyNode();
	}
}

bool FEventFlowDesignerApplicationMode::CanDesignerCopyNodes()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*FGraphPanelSelectionSet::TConstIterator(SelectedNodes)))
		{
			return Node->CanDuplicateNode();
		}
	}
	return true;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandPaste()
{
	const FVector2D PasteLocation = GetDesignerGraphEditor()->GetPasteLocation();

	UEdGraph* EdGraph = GetDesignerGraphEditor()->GetCurrentGraph();
	EdGraph->Modify();
	GetDesignerGraphEditor()->ClearSelectionSet();

	FString ExportedText;
	FPlatformApplicationMisc::ClipboardPaste(ExportedText);
	TSet<UEdGraphNode*> ImportedNodes;
	FEdGraphUtilities::ImportNodesFromText(EdGraph, ExportedText, ImportedNodes);

	FVector2D AvgNodePosition(0.0f, 0.0f);

	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	float InvNumNodes = 1.0f / float(ImportedNodes.Num());
	AvgNodePosition.X *= InvNumNodes;
	AvgNodePosition.Y *= InvNumNodes;

	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		GetDesignerGraphEditor()->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + PasteLocation.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + PasteLocation.Y;

		Node->SnapToGrid(16);

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}

	GetDesignerGraphEditor()->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}

}

bool FEventFlowDesignerApplicationMode::CanDesignerPasteNodes()
{
	return true;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandDuplicate()
{
	OnDesignerCommandCopy();
	OnDesignerCommandPaste();
}

bool FEventFlowDesignerApplicationMode::CanDesignerDuplicateNodes()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*FGraphPanelSelectionSet::TConstIterator(SelectedNodes)))
		{
			return Node->CanDuplicateNode();
		}
	}
	return true;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandDelete()
{
	GetDesignerGraphEditor()->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	GetDesignerGraphEditor()->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(SelectedNodes); It; ++It)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*It))
		{
			if (Node->CanUserDeleteNode())
			{
				Node->Modify();
				Node->DestroyNode();
			}
		}
	}
}

bool FEventFlowDesignerApplicationMode::CanDesignerDeleteNodes()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*FGraphPanelSelectionSet::TConstIterator(SelectedNodes)))
		{
			return Node->CanUserDeleteNode();
		}
	}
	return true;
}

#undef LOCTEXT_NAMESPACE