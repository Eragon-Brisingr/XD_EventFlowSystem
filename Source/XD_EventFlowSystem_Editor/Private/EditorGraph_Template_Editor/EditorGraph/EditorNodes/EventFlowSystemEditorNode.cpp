// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemEditorNode.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowGraph.h"
#include "EventFlowSystemEditorGraph.h"
#include "SEventFlowSystemGraphNode.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "EventFlowSystem_Editor_Log.h"
#include "GenericCommands.h"
#include "GraphEditorActions.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEventFlowSystemEditorNode::UEventFlowSystemEditorNode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer){}

const FName DefualtPinType("Default");
const FName InPinName("InPin");
const FName OutPinName("OutPin");

TSharedPtr<SGraphNode> UEventFlowSystemEditorNode::CreateVisualWidget()
{
	SlateNode= SNew(SEventFlowSystemGraphNode, this);
	return SlateNode;
}

TSharedPtr<SWidget> UEventFlowSystemEditorNode::GetContentWidget()
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedPtr<IDetailsView> View = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	View->SetObject(EventFlowBP_Node);
	return View;
}

void UEventFlowSystemEditorNode::UpdateVisualNode()
{
	if (SlateNode.IsValid())
	{
		SlateNode->UpdateGraphNode();
	}
}

void UEventFlowSystemEditorNode::SaveNodesAsChildren(TArray<UEdGraphNode*>& Children)
{
    for (UEdGraphNode* Child : Children)
    {
        UEventFlowSystemEditorNode* Node = Cast<UEventFlowSystemEditorNode>(Child);
        if (Node)
            EventFlowBP_Node->LinkArgumentNodeAsChild(Node->EventFlowBP_Node);
    }
}

bool UEventFlowSystemEditorNode::HasOutputPins()
{
	return EventFlowBP_Node ? EventFlowBP_Node->HasOutputPins() : true;
}

bool UEventFlowSystemEditorNode::HasInputPins()
{
	return EventFlowBP_Node ? EventFlowBP_Node->HasInputPins() : true;
}

void UEventFlowSystemEditorNode::AllocateDefaultPins()
{
	UEdGraphNode::AllocateDefaultPins();
	if (HasInputPins())
		CreatePin(EGPD_Input, DefualtPinType, InPinName);
	if (HasOutputPins())
		CreatePin(EGPD_Output, DefualtPinType, OutPinName);
}

FText UEventFlowSystemEditorNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (EventFlowBP_Node)
	{
		return FText::FromString(EventFlowBP_Node->GetName());
	}
	return FText::GetEmpty();
}

void UEventFlowSystemEditorNode::PrepareForCopying()
{
	if (EventFlowBP_Node)
		EventFlowBP_Node->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
	UEdGraphNode::PrepareForCopying();
}

void UEventFlowSystemEditorNode::DestroyNode()
{
	if (EventFlowBP_Node)
	{
		EventFlowBP_Node->GetGraph()->RemoveNode(EventFlowBP_Node);
		EventFlowBP_Node->ConditionalBeginDestroy();
		EventFlowBP_Node = nullptr;
	}
	UEdGraphNode::DestroyNode();
}

void UEventFlowSystemEditorNode::AutowireNewNode(UEdGraphPin * FromPin)
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


void UEventFlowSystemEditorNode::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
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

void UEventFlowSystemEditorNode::SetAssetNode(UEventFlowGraphNodeBase * InNode)
{
	EventFlowBP_Node = InNode;
}

UEventFlowGraphNodeBase* UEventFlowSystemEditorNode::GetAssetNode()
{
	return EventFlowBP_Node;
}

void UEventFlowSystemEditorNode::PostCopyNode()
{
	if (EventFlowBP_Node)
	{
		UEdGraph* EdGraph = GetGraph();
		UObject* ParentAsset = EdGraph ? EdGraph->GetOuter() : nullptr;
		EventFlowBP_Node->Rename(nullptr, ParentAsset, REN_DontCreateRedirectors | REN_DoNotDirty);
		EventFlowBP_Node->ClearFlags(RF_Transient);
	}
}

#undef LOCTEXT_NAMESPACE