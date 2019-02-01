﻿// Fill out your copyright notice in the Description page of Project Settings.
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
#include "CompilerResultsLog.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"

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
		ParentGraph->Modify();

		if (UXD_EventFlowSequenceBase* Sequence = Cast<UXD_EventFlowSequenceBase>(SequenceNode->EventFlowBpNode))
		{
			UEventElementEdNode* NewElementEdNode = NewObject<UEventElementEdNode>(ParentGraph, NAME_None, RF_Transactional);
			NewElementEdNode->OwingGraph = CastChecked<UEventFlowSystemEditorGraph>(ParentGraph);
			UXD_EventFlowElementBase* Element = NewObject<UXD_EventFlowElementBase>(Sequence, NewElementClass, NAME_None, RF_Transactional);
			Element->OwingEventFlowSequence = Sequence;
			NewElementEdNode->EventFlowBpNode = Element;
			NewElementEdNode->CreateNewGuid();
			NewElementEdNode->PostPlacedNewNode();
			NewElementEdNode->AllocateDefaultPins();

			SequenceNode->AddElement(NewElementEdNode);
		}
	}
	return nullptr;
}

UEdGraphNode* FNewBranch_SchemaAction::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	if (UXD_EventFlowSequenceBase* Sequence = Cast<UXD_EventFlowSequenceBase>(SequenceEdNode->EventFlowBpNode))
	{
		ParentGraph->Modify();

		UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = CastChecked<UEventFlowSystemEditorGraph>(ParentGraph);
		UXD_EventFlowElementBase* AssetNode = NewObject<UXD_EventFlowElementBase>(Sequence, NewElementClass, NAME_None, RF_Transactional);
		AssetNode->OwingEventFlowSequence = Sequence;
		FGraphNodeCreator<UEventSequenceBranch_SelectionEdNode> Creator(*ParentGraph);
		UEventSequenceBranch_SelectionEdNode* EditorNode = Creator.CreateNode(bSelectNewNode);
		EditorNode->EventFlowBpNode = AssetNode;
		Creator.Finalize();

		EditorNode->AutowireNewNode(FromPin);
		EditorNode->NodePosX = Location.X;
		EditorNode->NodePosY = Location.Y;

		return EditorNode;
	}
	return nullptr;
}

void UEventSequenceEdNodeBase::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
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
						TSharedPtr<FNewElement_SchemaAction> NewNodeAction = MakeShareable(new FNewElement_SchemaAction(ClassData.GetCategory(), FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments), 0, CastChecked<UEventSequenceEdNodeBase>(GraphNode), ClassData.GetClass()));

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
			.GraphNode(const_cast<UEventSequenceEdNodeBase*>(this))
			.AutoExpandActionMenu(true);

		MenuBuilder.AddWidget(Menu, FText(), true);
	}));
}

void UEventElementEdNode::DestroyNode()
{
	if (ParentNode)
	{
		ParentNode->RemoveElement(this);
	}
	if (OwingGraph)
	{
		OwingGraph->EventElements.Remove(this);
	}
	UEventFlowSystemEditorNodeBase::DestroyNode();
}

void UEventSequenceEdNodeBase::AddElement(UEventElementEdNode* Element)
{
	GetGraph()->Modify();
	Modify();

	Element->ParentNode = this;
	Element->SetFlags(RF_Transactional);

	Element->AutowireNewNode(nullptr);

	Element->NodePosX = 0;
	Element->NodePosY = 0;

	EventElements.Add(Element);

	CastChecked<UEventFlowSystemEditorGraph>(GetGraph())->EventElements.Add(Element);

	GetGraph()->NotifyGraphChanged();
}

void UEventSequenceEdNodeBase::RemoveElement(UEventElementEdNode* Element)
{
	EventElements.Remove(Element);
	Cast<UEventFlowSystemEditorGraph>(GetGraph())->EventElements.Remove(Element);
	Modify();

	GetGraph()->NotifyGraphChanged();
}

UXD_EventFlowSequenceBase* UEventSequenceEdNodeBase::BuildSequenceTree(UEventFlowGraphBlueprintGeneratedClass* Outer, FCompilerResultsLog& MessageLog) const
{
	if (EventFlowBpNode)
	{
		UXD_EventFlowSequenceBase** P_Sequence = Outer->SequenceList.FindByPredicate([&](UXD_EventFlowSequenceBase* E) {return E->GetName() == EventFlowBpNode->GetName(); });
		UXD_EventFlowSequenceBase* Sequence = P_Sequence ? *P_Sequence : nullptr;
		if (Sequence == nullptr)
		{
			Sequence = DuplicatedBpNode<UXD_EventFlowSequenceBase>(Outer);
			Outer->SequenceList.Add(Sequence);
			for (UEventElementEdNode* ElementEdNode : EventElements)
			{
				if (ElementEdNode->EventFlowBpNode)
				{
					Sequence->EventFlowElementList.Add(ElementEdNode->DuplicatedBpNode<UXD_EventFlowElementBase>(Sequence));
				}
				else
				{
					MessageLog.Error(TEXT("@@ 中任务元素 @@ 丢失"), this, ElementEdNode);
				}
			}
		}
		return Sequence;
	}
	else
	{
		MessageLog.Error(TEXT("@@ 中任务序列丢失"), this);
		return nullptr;
	}
}

TMap<TSubclassOf<UXD_EventFlowSequenceBase>, TSubclassOf<UEventSequenceEdNodeBase>> SequenceEdNodeMap
{
	{UEventFlowSequence_List::StaticClass(), UEventSequenceListEdNode::StaticClass()},
	{UEventFlowSequence_Branch::StaticClass(), UEventSequenceBranchEdNode::StaticClass()}
};

TSubclassOf<UEventSequenceEdNodeBase> UEventSequenceEdNodeBase::GetEdNodeClassByRuntimeClass(const TSubclassOf<UXD_EventFlowSequenceBase>& RunTimeSequence)
{
	return SequenceEdNodeMap[RunTimeSequence];
}

void UEventSequenceEdNodeBase::DestroyNode()
{
	Super::DestroyNode();

	for (UEventElementEdNode* Element : TArray<UEventElementEdNode*>(EventElements))
	{
		if (Element)
		{
			Element->DestroyNode();
		}
	}
}

UXD_EventFlowSequenceBase* UEventSequenceListEdNode::BuildSequenceTree(UEventFlowGraphBlueprintGeneratedClass* Outer, FCompilerResultsLog& MessageLog) const
{
	if (UEventFlowSequence_List* List = Cast<UEventFlowSequence_List>(Super::BuildSequenceTree(Outer, MessageLog)))
	{
		if (EventElements.Num() == 0)
		{
			MessageLog.Error(TEXT("@@ 至少需要存在一个任务元素"), this);
		}
		for (UEventSequenceEdNodeBase* NextSequenceEdNode : GetChildNodes<UEventSequenceEdNodeBase>())
		{
			List->NextSequenceTemplate = NextSequenceEdNode->BuildSequenceTree(Outer, MessageLog);
			break;
		}
		return List;
	}
	return nullptr;
}

FPinConnectionResponse UEventSequenceBranchEdNode::CanLinkedTo(const UEventFlowSystemEditorNodeBase* AnotherNode) const
{
	if (const UEventSequenceBranch_SelectionEdNode* BranchNode = Cast<const UEventSequenceBranch_SelectionEdNode>(AnotherNode))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
	}
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("只能和结束分支节点相连"));
}

bool UEventSequenceBranchEdNode::GetNodeLinkableContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
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
			TSharedPtr<FNewBranch_SchemaAction> NewNodeAction = MakeShareable(new FNewBranch_SchemaAction(ClassData.GetCategory(), FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments), 0, const_cast<UEventSequenceBranchEdNode*>(this), ClassData.GetClass()));
			BaseBuilder.AddAction(NewNodeAction);
		}
	}

	ContextMenuBuilder.Append(BaseBuilder);
	return true;
}

UXD_EventFlowSequenceBase* UEventSequenceBranchEdNode::BuildSequenceTree(UEventFlowGraphBlueprintGeneratedClass* Outer, FCompilerResultsLog& MessageLog) const
{
	if (UEventFlowSequence_Branch* Branch = Cast<UEventFlowSequence_Branch>(Super::BuildSequenceTree(Outer, MessageLog)))
	{
		TArray<UEventSequenceBranch_SelectionEdNode*> Branch_SelectionEdNodes = GetChildNodes<UEventSequenceBranch_SelectionEdNode>();
		if (Branch_SelectionEdNodes.Num() < 2)
		{
			MessageLog.Error(TEXT("@@ 分支数量必须大于等于2"), this);
		}
		for (UEventSequenceBranch_SelectionEdNode* BranchEdNode : Branch_SelectionEdNodes)
		{
			FEventFlowElementFinishWrapper FinishWarpper;
			FinishWarpper.EventFlowElement = BranchEdNode->DuplicatedBpNode<UXD_EventFlowElementBase>(Branch);
			for (UEventSequenceEdNodeBase* NextSequenceEdNode : BranchEdNode->GetChildNodes<UEventSequenceEdNodeBase>())
			{
				//TODO 根据边的Node设置Tag
				FinishWarpper.EventFlowFinishBranch.Add(NAME_None, NextSequenceEdNode->BuildSequenceTree(Outer, MessageLog));
			}
			Branch->EventFlowElementFinishList.Add(FinishWarpper);
		}
		return Branch;
	}
	return nullptr;
}

FPinConnectionResponse UEventSequenceBranch_SelectionEdNode::CanLinkedTo(const UEventFlowSystemEditorNodeBase* AnotherNode) const
{
	if (const UEventSequenceEdNodeBase* BranchNode = Cast<const UEventSequenceEdNodeBase>(AnotherNode))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, TEXT(""));
	}
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("只能和序列节点相连"));
}

#undef LOCTEXT_NAMESPACE