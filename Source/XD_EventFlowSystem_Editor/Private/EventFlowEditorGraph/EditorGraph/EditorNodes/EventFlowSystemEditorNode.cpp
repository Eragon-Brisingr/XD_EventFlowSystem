// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemEditorNode.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowSystemEditorGraph.h"
#include "SEventFlowSystemGraphNode.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "EventFlowSystem_Editor_Log.h"
#include "GenericCommands.h"
#include "GraphEditorActions.h"
#include "XD_EventFlowSequenceBase.h"
#include "XD_EventFlowSystem_Editor.h"
#include "EventFlowSystem_Editor_ClassHelper.h"
#include "XD_EventFlowElementBase.h"
#include "SGraphEditorActionMenuBase.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEventFlowSystemEditorNodeBase::UEventFlowSystemEditorNodeBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer){}

const FName DefualtPinType("Default");
const FName InPinName("InPin");
const FName OutPinName("OutPin");

TSharedPtr<SGraphNode> UEventFlowSystemEditorNodeBase::CreateVisualWidget()
{
	SlateNode= SNew(SEventFlowSystemGraphNode, this);
	return SlateNode;
}

TSharedPtr<SWidget> UEventFlowSystemEditorNodeBase::GetContentWidget()
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedPtr<IDetailsView> View = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	View->SetObject(EventFlowBpNode);
	return View;
}

void UEventFlowSystemEditorNodeBase::UpdateVisualNode()
{
	if (SlateNode.IsValid())
	{
		SlateNode->UpdateGraphNode();
	}
}

FPinConnectionResponse UEventFlowSystemEditorNodeBase::CanLinkedTo(const UEventFlowSystemEditorNodeBase* AnotherNode) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, TEXT(""));
}

bool UEventFlowSystemEditorNodeBase::GetNodeLinkableContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	return false;
}

void UEventFlowSystemEditorNodeBase::SaveNodesAsChildren(TArray<UEdGraphNode*>& Children)
{
    for (UEdGraphNode* Child : Children)
    {
        UEventFlowSystemEditorNodeBase* Node = Cast<UEventFlowSystemEditorNodeBase>(Child);
		if (Node)
		{
			EventFlowBpNode->LinkArgumentNodeAsChild(Node->EventFlowBpNode);
		}
    }
}

bool UEventFlowSystemEditorNodeBase::HasOutputPins()
{
	return true;
}

bool UEventFlowSystemEditorNodeBase::HasInputPins()
{
	return true;
}

void UEventFlowSystemEditorNodeBase::AllocateDefaultPins()
{
	UEdGraphNode::AllocateDefaultPins();
	if (HasInputPins())
		CreatePin(EGPD_Input, DefualtPinType, InPinName);
	if (HasOutputPins())
		CreatePin(EGPD_Output, DefualtPinType, OutPinName);
}

FText UEventFlowSystemEditorNodeBase::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (EventFlowBpNode)
	{
		return FText::FromString(EventFlowBpNode->GetName());
	}
	return FText::GetEmpty();
}

void UEventFlowSystemEditorNodeBase::PrepareForCopying()
{
	if (EventFlowBpNode)
		EventFlowBpNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
	UEdGraphNode::PrepareForCopying();
}

void UEventFlowSystemEditorNodeBase::DestroyNode()
{
	if (EventFlowBpNode)
	{
		//EventFlowBP_Node->GetGraph()->RemoveNode(EventFlowBP_Node);
		EventFlowBpNode->ConditionalBeginDestroy();
		EventFlowBpNode = nullptr;
	}
	UEdGraphNode::DestroyNode();
}

void UEventFlowSystemEditorNodeBase::AutowireNewNode(UEdGraphPin * FromPin)
{
	if (FromPin)
	{
		UEdGraphNode::AutowireNewNode(FromPin);
		if (FromPin->Direction == EEdGraphPinDirection::EGPD_Input)
		{
			if (GetSchema()->TryCreateConnection(FromPin, FindPin(OutPinName)))
			{
				FromPin->GetOwningNode()->NodeConnectionListChanged();
			}
		}
		if (FromPin->Direction == EEdGraphPinDirection::EGPD_Output)
		{
			if (GetSchema()->TryCreateConnection(FromPin, FindPin(InPinName)))
			{
				FromPin->GetOwningNode()->NodeConnectionListChanged();
			}
		}
	}
}


void UEventFlowSystemEditorNodeBase::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
{
	FMenuBuilder* MenuBuilder = Context.MenuBuilder;
	MenuBuilder->BeginSection(NAME_None, LOCTEXT("NodeActionsMenuHeader", "节点操作"));
	{
		MenuBuilder->AddMenuEntry(FGenericCommands::Get().Delete);
		MenuBuilder->AddMenuEntry(FGenericCommands::Get().Cut);
		MenuBuilder->AddMenuEntry(FGenericCommands::Get().Copy);
		MenuBuilder->AddMenuEntry(FGenericCommands::Get().Duplicate);

		MenuBuilder->AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
	}
	MenuBuilder->EndSection();
}

void UEventFlowSystemEditorNodeBase::SetAssetNode(UEventFlowGraphNodeBase * InNode)
{
	EventFlowBpNode = InNode;
}

UEventFlowGraphNodeBase* UEventFlowSystemEditorNodeBase::GetAssetNode()
{
	return EventFlowBpNode;
}

void UEventFlowSystemEditorNodeBase::PostCopyNode()
{
	if (EventFlowBpNode)
	{
		UEdGraph* EdGraph = GetGraph();
		UObject* ParentAsset = EdGraph ? EdGraph->GetOuter() : nullptr;
		EventFlowBpNode->Rename(nullptr, ParentAsset, REN_DontCreateRedirectors | REN_DoNotDirty);
		EventFlowBpNode->ClearFlags(RF_Transient);
	}
}


FText UEventFlowSystemStartEdNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("事件开始", "事件开始");
}

UEdGraphNode* FNewElement_SchemaAction::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	if (SequenceNode)
	{
		UEventElementEdNode* NewElementEdNode = NewObject<UEventElementEdNode>(ParentGraph, NAME_None, RF_Transactional);
		NewElementEdNode->EventFlowBpNode = NewObject<UXD_EventFlowElementBase>(SequenceNode, NewElementClass, NAME_None, RF_Transactional);
		SequenceNode->AddElement(NewElementEdNode);
	}
	return nullptr;
}

UEdGraphNode* FNewBranch_SchemaAction::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	check(ParentGraph);

	ParentGraph->Modify();

	UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = CastChecked<UEventFlowSystemEditorGraph>(ParentGraph);

	UXD_EventFlowElementBase* AssetNode = NewObject<UXD_EventFlowElementBase>(EventFlowSystemEditorGraph->GetBlueprint(), NewElementClass, NAME_None, RF_Transactional);
	FGraphNodeCreator<UEventSequenceBranchEdNode> Creator(*ParentGraph);
	UEventSequenceBranchEdNode* EditorNode = Creator.CreateNode(bSelectNewNode);
	EditorNode->EventFlowBpNode = AssetNode;
	Creator.Finalize();

	//EditorNode->AllocateDefaultPins();   for some reason it was called 2 times even if I only call it here
	EditorNode->AutowireNewNode(FromPin);
	EditorNode->NodePosX = Location.X;
	EditorNode->NodePosY = Location.Y;

	return EditorNode;
}

void UEventSequenceEdNode::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
{
	UEventFlowSystemEditorNodeBase::GetContextMenuActions(Context);

	Context.MenuBuilder->AddSubMenu(
		LOCTEXT("添加事件元素", "添加事件元素"),
		LOCTEXT("添加事件元素", "添加事件元素"),
		FNewMenuDelegate::CreateLambda(
	[=](class FMenuBuilder& MenuBuilder)
	{
		class SGraphEditorActionMenu_EventElement : public SGraphEditorActionMenuBase
		{
		public:
			void CollectAllActions(FGraphActionListBuilderBase& OutAllActions) override
			{
				FGraphContextMenuBuilder ContextMenuBuilder(GraphObj);

				FCategorizedGraphActionListBuilder BaseBuilder(TEXT("事件元素"));

				FXD_EventFlowSystem_EditorModule& Module = FModuleManager::LoadModuleChecked<FXD_EventFlowSystem_EditorModule>("XD_EventFlowSystem_Editor");
				TSharedPtr<FEventFlowSystem_Editor_ClassHelper> Helper = Module.GetHelper();
				TArray<FEventFlowSystem_Editor_ClassData> AllSubClasses;
				Helper->GatherClasses(UXD_EventFlowElementBase::StaticClass(), AllSubClasses);

				FText ToolTip = LOCTEXT("NewEditorGraphNodeTooltip", "Add a {NodeName} to the graph.");
				FText MenuDesc = LOCTEXT("NewEditorGraphNodeDescription", "{NodeName}");
				for (auto& ClassData : AllSubClasses)
				{
					if (!ClassData.GetClass()->HasAnyClassFlags(CLASS_Abstract))
					{
						FFormatNamedArguments Arguments;
						Arguments.Add(TEXT("NodeName"), ClassData.GetClass()->GetDisplayNameText());
						TSharedPtr<FNewElement_SchemaAction> NewNodeAction = MakeShareable(new FNewElement_SchemaAction(ClassData.GetCategory(), FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments), 0, CastChecked<UEventSequenceEdNode>(GraphNode), ClassData.GetClass()));

						BaseBuilder.AddAction(NewNodeAction);
					}
				}

				ContextMenuBuilder.Append(BaseBuilder);

				OutAllActions.Append(ContextMenuBuilder);
			}
		};

		TSharedRef<SGraphEditorActionMenu_EventElement> Menu =
			SNew(SGraphEditorActionMenu_EventElement)
			.GraphObj(const_cast<UEdGraph*>(Context.Graph))
			.GraphNode(const_cast<UEventSequenceEdNode*>(this))
			.AutoExpandActionMenu(true);

		MenuBuilder.AddWidget(Menu, FText(), true);
	}));
}

FPinConnectionResponse UEventSequenceEdNode::CanLinkedTo(const UEventFlowSystemEditorNodeBase* AnotherNode) const
{
	if (UEventFlowSequence_Branch* Branch = Cast<UEventFlowSequence_Branch>(EventFlowBpNode))
	{
		if (const UEventSequenceBranchEdNode* BranchNode = Cast<const UEventSequenceBranchEdNode>(AnotherNode))
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
		}
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("只能和结束分支节点相连"));
	}
	return Super::CanLinkedTo(AnotherNode);
}

void UEventElementEdNode::DestroyNode()
{
	if (ParentNode)
	{
		ParentNode->RemoveElement(this);
	}
	UEventFlowSystemEditorNodeBase::DestroyNode();
}

bool UEventSequenceEdNode::GetNodeLinkableContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	if (UEventFlowSequence_Branch* Branch = Cast<UEventFlowSequence_Branch>(EventFlowBpNode))
	{
		FCategorizedGraphActionListBuilder BaseBuilder(TEXT("事件结束分支"));

		FXD_EventFlowSystem_EditorModule& Module = FModuleManager::LoadModuleChecked<FXD_EventFlowSystem_EditorModule>("XD_EventFlowSystem_Editor");
		TSharedPtr<FEventFlowSystem_Editor_ClassHelper> Helper = Module.GetHelper();
		TArray<FEventFlowSystem_Editor_ClassData> AllSubClasses;
		Helper->GatherClasses(UXD_EventFlowElementBase::StaticClass(), AllSubClasses);

		FText ToolTip = LOCTEXT("NewEditorGraphNodeTooltip", "Add a {NodeName} to the graph.");
		FText MenuDesc = LOCTEXT("NewEditorGraphNodeDescription", "{NodeName}");
		for (auto& ClassData : AllSubClasses)
		{
			if (!ClassData.GetClass()->HasAnyClassFlags(CLASS_Abstract))
			{
				FFormatNamedArguments Arguments;
				Arguments.Add(TEXT("NodeName"), ClassData.GetClass()->GetDisplayNameText());
				TSharedPtr<FNewBranch_SchemaAction> NewNodeAction;

				NewNodeAction = MakeShareable(new FNewBranch_SchemaAction(ClassData.GetCategory(), FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments), 0, ClassData.GetClass()));

				BaseBuilder.AddAction(NewNodeAction);
			}
		}

		ContextMenuBuilder.Append(BaseBuilder);
		return true;
	}
	return false;
}

void UEventSequenceEdNode::AddElement(UEventElementEdNode* Element)
{
	GetGraph()->Modify();
	Modify();

	Element->SetFlags(RF_Transactional);

	Element->CreateNewGuid();
	Element->PostPlacedNewNode();
	Element->AllocateDefaultPins();
	Element->AutowireNewNode(nullptr);

	Element->NodePosX = 0;
	Element->NodePosY = 0;

	Elements.Add(Element);

	GetGraph()->NotifyGraphChanged();
}

void UEventSequenceEdNode::RemoveElement(UEventElementEdNode* Element)
{
	Elements.RemoveSingle(Element);
	Modify();

	GetGraph()->NotifyGraphChanged();
}

FPinConnectionResponse UEventSequenceBranchEdNode::CanLinkedTo(const UEventFlowSystemEditorNodeBase* AnotherNode) const
{
	if (const UEventSequenceEdNode* BranchNode = Cast<const UEventSequenceEdNode>(AnotherNode))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, TEXT(""));
	}
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("只能和序列节点相连"));
}

#undef LOCTEXT_NAMESPACE