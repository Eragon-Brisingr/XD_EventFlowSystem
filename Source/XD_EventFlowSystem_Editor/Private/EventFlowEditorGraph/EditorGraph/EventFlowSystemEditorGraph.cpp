// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemEditorGraph.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowSystemEditorNode.h"
#include "EventFlowSystem_Editor_Log.h"
#include "EdGraph/EdGraphPin.h"
#include "EventFlowGraphBlueprint.h"

void UEventFlowSystemEditorGraph::BuildSubobjectMapping(UObject* OtherObject, TMap<UObject*, UObject*>& ObjectMapping) const
{
#if WITH_EDITORONLY_DATA
	UEdGraph* OtherGraph = CastChecked<UEdGraph>(OtherObject);

	auto FindMatchingNode = [](UEdGraph* InGraphToSearch, UEdGraphNode* InNodeToFind) -> UEdGraphNode*
	{
		TArray<UEdGraphNode*, TInlineAllocator<8>> PotentialMatches;

		for (UEdGraphNode* GraphNode : InGraphToSearch->Nodes)
		{
			if (GraphNode->GetClass() == InNodeToFind->GetClass())
			{
				// Ideally the node would match by GUID or name
				if (GraphNode->NodeGuid == InNodeToFind->NodeGuid || GraphNode->GetFName() == InNodeToFind->GetFName())
				{
					return GraphNode;
				}

				// If it doesn't match by either, we may be searching an older graph without stable node IDs... consider the node title as significant in this case
				{
					const FText Title1 = GraphNode->GetNodeTitle(ENodeTitleType::FullTitle);
					const FText Title2 = InNodeToFind->GetNodeTitle(ENodeTitleType::FullTitle);

					if (Title1.ToString().Equals(Title2.ToString(), ESearchCase::CaseSensitive))
					{
						PotentialMatches.Emplace(GraphNode);
					}
				}
			}
		}

		// Only use the title-based resolution if we found a single match!
		if (PotentialMatches.Num() == 1)
		{
			return PotentialMatches[0];
		}

		return nullptr;
	};

	for (UEdGraphNode* GraphNode : EventElements)
	{
		if (!ObjectMapping.Contains(GraphNode))
		{
			UEdGraphNode* OtherGraphNode = FindMatchingNode(OtherGraph, GraphNode);
			ObjectMapping.Emplace(GraphNode, OtherGraphNode);
		}
	}
#endif

	Super::BuildSubobjectMapping(OtherObject, ObjectMapping);
}

void UEventFlowSystemEditorGraph::RefreshNodes()
{
	for (UEdGraphNode* Node : Nodes)
	{
		if (UEventFlowSystemEditorNodeBase* EventFlowEdNode = Cast<UEventFlowSystemEditorNodeBase>(Node))
		{
			EventFlowEdNode->UpdateVisualNode();
		}
	}
}

UXD_EventFlowSequenceBase* UEventFlowSystemEditorGraph::BuildSequenceTreeInstance(UEventFlowGraphBlueprintGeneratedClass* Outer, FCompilerResultsLog& MessageLog) const
{
	for (UEventSequenceEdNodeBase* SequenceEditorNode : StartNode->GetChildNodes<UEventSequenceEdNodeBase>())
	{
		return SequenceEditorNode->BuildSequenceTree(Outer, MessageLog);
	}
	return nullptr;
}

UEventFlowGraphBlueprint* UEventFlowSystemEditorGraph::GetBlueprint() const
{
	return CastChecked<UEventFlowGraphBlueprint>(GetOuter());
}

TArray<class UEventFlowSystemEditorNodeBase*> UEventFlowSystemEditorGraph::GetAllNodes() const
{
	TArray<class UEventFlowSystemEditorNodeBase*> Res;
	for (UEdGraphNode* Node : Nodes)
	{
		if (UEventFlowSystemEditorNodeBase* EdNode = Cast<UEventFlowSystemEditorNodeBase>(Node))
		{
			Res.Add(EdNode);
		}
	}
	Res.Append(EventElements);
	return Res;
}
