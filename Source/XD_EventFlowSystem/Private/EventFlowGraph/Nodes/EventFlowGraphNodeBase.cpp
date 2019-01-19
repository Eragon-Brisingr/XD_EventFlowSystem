// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowGraphNodeBase.h"
#include "EventFlowGraph.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEventFlowGraphNodeBase::UEventFlowGraphNodeBase()
{
	
}

UEventFlowGraphNodeBase * UEventFlowGraphNodeBase::GetNodePointer_Implementation()
{
	return this;
}

void UEventFlowGraphNodeBase::SetGraph(UEventFlowGraph * InGraph)
{
	Graph = InGraph;
}

UEventFlowGraph * UEventFlowGraphNodeBase::GetGraph()
{
	return Graph;
}

FText UEventFlowGraphNodeBase::GetNodeTitle_Implementation()
{
#if WITH_EDITOR
	return GetClass()->GetDisplayNameText();
#else
	return FText::GetEmpty();
#endif
}

TArray<UEventFlowGraphNodeBase*> UEventFlowGraphNodeBase::GetChildren()
{
	TArray<UEventFlowGraphNodeBase*>ReturnArray;
	for (UEventFlowGraphNodeBase* Node : ChildNodes)
	{
        auto* object = Node->GetNodePointer();
		if(object)ReturnArray.Add(object);
	}
	return ReturnArray;
}

TArray<UEventFlowGraphNodeBase*> UEventFlowGraphNodeBase::GetParents()
{
	return ParentNodes;
}

bool UEventFlowGraphNodeBase::HasInputPins_Implementation()
{
	return bHasInputPins;
}

bool UEventFlowGraphNodeBase::HasOutputPins_Implementation()
{
	return bHasOutputPins;
}

void UEventFlowGraphNodeBase::AddToChildren(UEventFlowGraphNodeBase * NewSubNode)
{
	ChildNodes.Add(NewSubNode);
}

void UEventFlowGraphNodeBase::AddToParent(UEventFlowGraphNodeBase * NewParentNode)
{
	ParentNodes.Add(NewParentNode);
}

bool UEventFlowGraphNodeBase::HasParentNodes()
{
	return ParentNodes.Num() > 0;
}

void UEventFlowGraphNodeBase::LinkArgumentNodeAsChild(UEventFlowGraphNodeBase * Child)
{
	AddToChildren(Child);
	Child->AddToParent(this);
}

void UEventFlowGraphNodeBase::ClearLinks()
{
	ParentNodes.Empty();
	ChildNodes.Empty();
}

bool UEventFlowGraphNodeBase::RemoveLinkedNode(UEventFlowGraphNodeBase * NodeToRemove)
{
	return RemoveNodeFromParents(NodeToRemove) || RemoveNodeFromChilds(NodeToRemove);
}

bool UEventFlowGraphNodeBase::RemoveNodeFromParents(UEventFlowGraphNodeBase * NodeToRemove)
{
	return ParentNodes.Remove(NodeToRemove);
}

bool UEventFlowGraphNodeBase::RemoveNodeFromChilds(UEventFlowGraphNodeBase * NodeToRemove)
{
	return ChildNodes.Remove(NodeToRemove);
}

#undef LOCTEXT_NAMESPACE