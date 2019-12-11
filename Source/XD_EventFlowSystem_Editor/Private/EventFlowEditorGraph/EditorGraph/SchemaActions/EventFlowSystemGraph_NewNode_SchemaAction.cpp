// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowEditorGraph/EditorGraph/SchemaActions/EventFlowSystemGraph_NewNode_SchemaAction.h"
#include "EventFlowEditorGraph/EditorGraph/EditorNodes/EventFlowSystemEditorNode.h"
#include <EdGraph/EdGraph.h>
#include "Events/XD_EventFlowSequenceBase.h"
#include "EventFlowEditorGraph/EditorGraph/EventFlowSystemEditorGraph.h"
#include "XD_TemplateLibrary.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEdGraphNode* FNewSequence_SchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	check(ParentGraph);

	ParentGraph->Modify();

	UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = CastChecked<UEventFlowSystemEditorGraph>(ParentGraph);

	struct FGraphNodeCreatorByClass
	{
		FGraphNodeCreatorByClass(const TSubclassOf<UEdGraphNode>& NodeType, UEdGraph& InGraph)
			:NodeType(NodeType), InGraph(InGraph)
		{}

		UEdGraphNode* CreateNode(bool bSelectNewNode = true)
		{
			struct UEdGraphHelper : public UEdGraph
			{
				static UEdGraphNode* CreateNode(UEdGraph* Graph, TSubclassOf<UEdGraphNode> NewNodeClass, bool bSelectNewNode = true)
				{
					return static_cast<UEdGraphHelper*>(Graph)->UEdGraph::CreateNode(NewNodeClass, bSelectNewNode);
				}
			};
			Node = UEdGraphHelper::CreateNode(&InGraph, NodeType, bSelectNewNode);
			return Node;
		}

		TSubclassOf<UEdGraphNode> NodeType;

		void Finalize()
		{
			Node->CreateNewGuid();
			Node->PostPlacedNewNode();
			if (Node->Pins.Num() == 0)
			{
				Node->AllocateDefaultPins();
			}
		}
	private:
		UEdGraph& InGraph;
		UEdGraphNode* Node;
	};

	UXD_EventFlowSequenceBase* AssetNode = NewObject<UXD_EventFlowSequenceBase>(EventFlowSystemEditorGraph->GetBlueprint(), NewSequenceClass, NAME_None, RF_Transactional);
	FGraphNodeCreatorByClass Creator(UEventSequenceEdNodeBase::GetEdNodeClassByRuntimeClass(NewSequenceClass), *ParentGraph);
	UEventSequenceEdNodeBase* EditorNode = (UEventSequenceEdNodeBase*)Creator.CreateNode(bSelectNewNode);
	EditorNode->EventFlowBpNode = AssetNode;
	Creator.Finalize();

	//EditorNode->AllocateDefaultPins();   for some reason it was called 2 times even if I only call it here
	EditorNode->AutowireNewNode(FromPin);
	EditorNode->NodePosX = Location.X;
	EditorNode->NodePosY = Location.Y;

	return EditorNode;
}

#undef LOCTEXT_NAMESPACE