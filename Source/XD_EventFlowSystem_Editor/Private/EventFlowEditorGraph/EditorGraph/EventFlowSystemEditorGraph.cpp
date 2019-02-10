// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemEditorGraph.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowSystemEditorNode.h"
#include "EventFlowSystem_Editor_Log.h"
#include "EdGraph/EdGraphPin.h"
#include "EventFlowGraphBlueprint.h"

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

TArray<UEventFlowSystemEditorNodeBase*> UEventFlowSystemEditorGraph::GetAllNodes() const
{
	TArray<UEventFlowSystemEditorNodeBase*> Res;
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

TArray<UEventFlowSystemEditorNodeBase*> UEventFlowSystemEditorGraph::GetAllRootLinkedNodes() const
{
	struct FLinkedNodeCollector
	{
		static void Collect(UEventFlowSystemEditorNodeBase* Root, TArray<UEventFlowSystemEditorNodeBase*>& Res)
		{
			TArray<UEventFlowSystemEditorNodeBase*> Childs = Root->GetChildNodes<UEventFlowSystemEditorNodeBase>();
			Res.Append(Childs);
			for (UEventFlowSystemEditorNodeBase* Child : Childs)
			{
				Collect(Child, Res);
			}
		}
	};

	TArray<UEventFlowSystemEditorNodeBase*> Res;
	FLinkedNodeCollector::Collect(StartNode, Res);

	for (UEventElementEdNode* Element : EventElements)
	{
		if (Res.Contains(Element->ParentNode))
		{
			Res.Add(Element);
		}
	}
	return Res;
}
