// Fill out your copyright notice in the Description page of Project Settings.

#include "SEventFlowDetailsView.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "SComboButton.h"
#include "DetailWidgetRow.h"
#include "ObjectEditorUtils.h"
#include "KismetEditorUtilities.h"
#include "SWidgetSwitcher.h"
#include "IDetailPropertyExtensionHandler.h"
#include "SEventFlowPropertyBinding.h"
#include "EventFlowSystemEditor.h"
#include "EventFlowGraphBlueprint.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowGraph"

void SEventFlowDetailsView::Construct(const FArguments& InArgs, TWeakPtr<FEventFlowSystemEditor> InEditor)
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

	FEventFlowSystemEditor* EventFlowSystemEditor = Editor.Pin().Get();
	PropertyView->OnFinishedChangingProperties().AddLambda([=](const FPropertyChangedEvent&) { EventFlowSystemEditor->GetBlueprintObj()->Status = EBlueprintStatus::BS_Dirty; });
	PropertyView->SetExtensionHandler(MakeShareable(new FEventFlowDetailExtensionHandler(EventFlowSystemEditor)));

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

	FEventFlowSystemEditor* BlueprintEditor = EventFlowSystemEditor;
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

FText SEventFlowDetailsView::GetNameText() const
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

bool SEventFlowDetailsView::HandleVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
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

void SEventFlowDetailsView::HandleNameTextChanged(const FText& Text)
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

void SEventFlowDetailsView::HandleNameTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	FEventFlowSystemEditor* EventFlowSystemEditor = Editor.Pin().Get();
	UEventFlowGraphBlueprint* Blueprint = EventFlowSystemEditor->GetEventFlowBlueprint();

	const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
	if (Selections.Num() == 1)
	{
		if (UEventFlowGraphNodeBase* Node = Cast<UEventFlowGraphNodeBase>(Selections[0].Get()))
		{
			const TCHAR* NewName = *Text.ToString();

			if (Node->Rename(NewName, nullptr, REN_Test))
			{
				TMap<UEventFlowGraphNodeBase*, FString> NodeAndOldNames;
				if (UXD_EventFlowSequenceBase* EventFlowSequence = Cast<UXD_EventFlowSequenceBase>(Node))
				{
					for (UEventElementEdNode* EventElement : EventFlowSystemEditor->GetEditorGraph()->EventElements)
					{
						if (UXD_EventFlowElementBase* SubNode = Cast<UXD_EventFlowElementBase>(EventElement->EventFlowBpNode))
						{
							if (SubNode->bIsVariable && SubNode->OwingEventFlowSequence == EventFlowSequence)
							{
								NodeAndOldNames.Add(SubNode, SubNode->GetVarRefName());
							}
						}
					}
				}
				if (Node->bIsVariable)
				{
					NodeAndOldNames.Add(Node, Node->GetVarRefName());
				}
				Node->Rename(NewName);

				if (NodeAndOldNames.Num() > 0)
				{
					for (const TPair<UEventFlowGraphNodeBase*, FString>& NodeAndOldName : NodeAndOldNames)
					{
						FBlueprintEditorUtils::ReplaceVariableReferences(Blueprint, *NodeAndOldName.Value, *NodeAndOldName.Key->GetVarRefName());
						FBlueprintEditorUtils::ValidateBlueprintChildVariables(Blueprint, *NodeAndOldName.Key->GetVarRefName());
					}
					FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
				}

				FEventFlowDelegateEditorBinding* Binding = Blueprint->Bindings.FindByPredicate([&](const FEventFlowDelegateEditorBinding& E) {return E.Object.Get() == Node; });

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

ECheckBoxState SEventFlowDetailsView::GetIsVariable() const
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

void SEventFlowDetailsView::HandleIsVariableChanged(ECheckBoxState CheckState)
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

#undef LOCTEXT_NAMESPACE
