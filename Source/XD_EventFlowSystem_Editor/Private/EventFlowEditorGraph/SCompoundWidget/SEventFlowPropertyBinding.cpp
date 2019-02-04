// Fill out your copyright notice in the Description page of Project Settings.

#include "SEventFlowPropertyBinding.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "SComboButton.h"
#include "DetailWidgetRow.h"
#include "ObjectEditorUtils.h"
#include "KismetEditorUtilities.h"
#include "SWidgetSwitcher.h"
#include "IDetailPropertyExtensionHandler.h"
#include "EventFlowGraphBlueprint.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowGraph"

void SEventFlowPropertyBinding::Construct(const FArguments& InArgs, FEventFlowSystemEditor* InEditor, UDelegateProperty* DelegateProperty, TSharedRef<IPropertyHandle> Property)
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
			.OnGetMenuContent(this, &SEventFlowPropertyBinding::OnGenerateDelegateMenu, Object, Property)
			.ContentPadding(1)
			.ButtonContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(this, &SEventFlowPropertyBinding::GetCurrentBindingImage, Property)
					.ColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.25f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 1, 0, 0)
				[
					SNew(STextBlock)
					.Text(this, &SEventFlowPropertyBinding::GetCurrentBindingText, Property)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
			.Visibility(this, &SEventFlowPropertyBinding::GetGotoBindingVisibility, Property)
			.OnClicked(this, &SEventFlowPropertyBinding::HandleGotoBindingClicked, Property)
			.VAlign(VAlign_Center)
			.ToolTipText(LOCTEXT("GotoFunction", "Goto Function"))
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Browse"))
			]
		]
	];
}

TSharedRef<SWidget> SEventFlowPropertyBinding::OnGenerateDelegateMenu(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle)
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
				FUIAction(FExecuteAction::CreateSP(this, &SEventFlowPropertyBinding::HandleRemoveBinding, PropertyHandle))
			);
		}
		else
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("CreateBinding", "添加绑定"),
				LOCTEXT("CreateBindingToolTip", "在当前的属性上创建函数绑定"),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "Plus"),
				FUIAction(FExecuteAction::CreateSP(this, &SEventFlowPropertyBinding::HandleCreateAndAddBinding, Object, PropertyHandle))
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

const FSlateBrush* SEventFlowPropertyBinding::GetCurrentBindingImage(TSharedRef<IPropertyHandle> PropertyHandle) const
{
	static FName FunctionIcon(TEXT("GraphEditor.Function_16x"));
	return FEditorStyle::GetBrush(FunctionIcon);
}

FText SEventFlowPropertyBinding::GetCurrentBindingText(TSharedRef<IPropertyHandle> PropertyHandle) const
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

EVisibility SEventFlowPropertyBinding::GetGotoBindingVisibility(TSharedRef<IPropertyHandle> PropertyHandle) const
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

FReply SEventFlowPropertyBinding::HandleGotoBindingClicked(TSharedRef<IPropertyHandle> PropertyHandle)
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

bool SEventFlowPropertyBinding::CanRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle) const
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

void SEventFlowPropertyBinding::HandleRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle)
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

void SEventFlowPropertyBinding::HandleCreateAndAddBinding(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle)
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

void SEventFlowPropertyBinding::GotoFunction(UEdGraph* FunctionGraph)
{
	Editor->SetCurrentMode(FBlueprintApplicationModesTemplate::GraphMode);

	Editor->OpenDocument(FunctionGraph, FDocumentTracker::OpenNewDocument);
}

#undef LOCTEXT_NAMESPACE

bool FEventFlowDetailExtensionHandler::IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& InPropertyHandle) const
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

TSharedRef<SWidget> FEventFlowDetailExtensionHandler::GenerateExtensionWidget(const UClass* InObjectClass, TSharedPtr<IPropertyHandle> InPropertyHandle)
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

	return SNew(SEventFlowPropertyBinding, BlueprintEditor, DelegateProperty, InPropertyHandle.ToSharedRef());
}
