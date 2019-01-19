// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemGraph_NewNode_SchemaAction.h"
#include "EventFlowGraph.h"
#include "EventFlowSystemEditorNode.h"
#include "EdGraph/EdGraph.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEdGraphNode * FEventFlowSystemGraph_NewNode_SchemaAction::PerformAction(UEdGraph * ParentGraph, UEdGraphPin * FromPin, const FVector2D Location, bool bSelectNewNode)
{
	check(ParentGraph);

	ParentGraph->Modify();


	UEventFlowGraph* EventFlowGraph = CastChecked<UEventFlowGraph>(ParentGraph->GetOuter());
	EventFlowGraph->Modify();

	UEventFlowGraphNodeBase* AssetNode = EventFlowGraph->SpawnNodeInsideGraph<UEventFlowGraphNodeBase>(NewNodeClass);

	UEdGraphNode* EditorNode = CreateEditorNode(ParentGraph, bSelectNewNode, AssetNode);

	//EditorNode->AllocateDefaultPins();   for some reason it was called 2 times even if I only call it here
	EditorNode->AutowireNewNode(FromPin);
	EditorNode->NodePosX = Location.X;
	EditorNode->NodePosY = Location.Y;

	return EditorNode;
}

UEdGraphNode * FEventFlowSystemGraph_NewNode_SchemaAction::CreateEditorNode(UEdGraph * ParentGraph, bool bSelectNewNode, UEventFlowGraphNodeBase* AssetNode)
{
    FGraphNodeCreator<UEventFlowSystemEditorNode>Creator(*ParentGraph);
    UEventFlowSystemEditorNode* EdNode = Creator.CreateNode(bSelectNewNode);
    EdNode->SetAssetNode(AssetNode);
    Creator.Finalize();
    return EdNode;
}
#undef LOCTEXT_NAMESPACE