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
	for (UEventSequenceEdNode* SequenceEditorNode : StartNode->GetChildNodes<UEventSequenceEdNode>())
	{
		return SequenceEditorNode->BuildSequenceTree(Outer, MessageLog);
	}
	return nullptr;
}

UEventFlowGraphBlueprint* UEventFlowSystemEditorGraph::GetBlueprint() const
{
	return CastChecked<UEventFlowGraphBlueprint>(GetOuter());
}
