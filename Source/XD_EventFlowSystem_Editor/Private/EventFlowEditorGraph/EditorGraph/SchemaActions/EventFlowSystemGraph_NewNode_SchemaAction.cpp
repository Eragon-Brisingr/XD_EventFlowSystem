// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemGraph_NewNode_SchemaAction.h"
#include "EventFlowSystemEditorNode.h"
#include "EdGraph/EdGraph.h"
#include "XD_EventFlowSequenceBase.h"
#include "EventFlowSystemEditorGraph.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEdGraphNode* FNewSequence_SchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	check(ParentGraph);

	ParentGraph->Modify();

	UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = CastChecked<UEventFlowSystemEditorGraph>(ParentGraph);

	UXD_EventFlowSequenceBase* AssetNode = NewObject<UXD_EventFlowSequenceBase>(EventFlowSystemEditorGraph->GetBlueprint(), NewSequenceClass, NAME_None, RF_Transactional);
	FGraphNodeCreator<UEventSequenceEdNode> Creator(*ParentGraph);
	UEventSequenceEdNode* EditorNode = Creator.CreateNode(bSelectNewNode);
	EditorNode->EventFlowBpNode = AssetNode;
	Creator.Finalize();

	//EditorNode->AllocateDefaultPins();   for some reason it was called 2 times even if I only call it here
	EditorNode->AutowireNewNode(FromPin);
	EditorNode->NodePosX = Location.X;
	EditorNode->NodePosY = Location.Y;

	return EditorNode;
}

#undef LOCTEXT_NAMESPACE