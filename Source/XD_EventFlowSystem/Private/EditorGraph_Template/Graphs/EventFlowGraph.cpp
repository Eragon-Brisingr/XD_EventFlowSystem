// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowGraph.h"
#include "EventFlowGraphNodeBase.h"
#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEventFlowGraph::UEventFlowGraph()
{
    
#if WITH_EDITORONLY_DATA
	EdGraph = nullptr;
#endif

}

void UEventFlowGraph::InitGraph(UObject * ParentObject)
{
    Owner = ParentObject;
}

#if WITH_EDITORONLY_DATA
void UEventFlowGraph::AddNode(UEventFlowGraphNodeBase * InNode)
{
	InNode->SetGraph(this);
	InNode->Rename(nullptr, this);
	int32 Index = Nodes.Add(InNode);
}

void UEventFlowGraph::RemoveNode(UEventFlowGraphNodeBase * NodeToRemove)
{
	for (UEventFlowGraphNodeBase* Node : Nodes)
	{
		Node->RemoveLinkedNode(NodeToRemove);
	}

	int32 Removed=Nodes.Remove(NodeToRemove);
}
#endif

#undef LOCTEXT_NAMESPACE