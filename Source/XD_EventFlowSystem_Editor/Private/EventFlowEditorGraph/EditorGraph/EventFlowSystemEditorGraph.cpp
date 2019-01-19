// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemEditorGraph.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowSystemEditorNode.h"
#include "EventFlowSystem_Editor_Log.h"
#include "EdGraph/EdGraphPin.h"
#include "EventFlowGraph.h"

void UEventFlowSystemEditorGraph::BuildGraph()
{
	LinkAssetNodes();
	MapNamedNodes();
}

void UEventFlowSystemEditorGraph::ClearOldLinks()
{
	for (UEdGraphNode* EditorNode : Nodes)
	{
		UEventFlowSystemEditorNode* EdNode = Cast<UEventFlowSystemEditorNode>(EditorNode);
		if (EdNode && EdNode->EventFlowBP_Node)
		{
			EdNode->EventFlowBP_Node->ClearLinks();
		}
	}
}

void UEventFlowSystemEditorGraph::LinkAssetNodes()
{
	ClearOldLinks();
	EventFlowSystem_Log("Starting to link all asset nodes from the editor graph links.");
	for (UEdGraphNode* EditorNode : Nodes)
	{
		if (UEventFlowSystemEditorNode* EdNode = Cast<UEventFlowSystemEditorNode>(EditorNode))
		{
			UEventFlowGraphNodeBase* NodeAsset = EdNode->EventFlowBP_Node;
			if (NodeAsset != nullptr)
			{

				TArray<UEdGraphPin*>& EdPinsParent = EdNode->Pins;
				TArray<UEdGraphNode*>Children;

				for (UEdGraphPin* Pin : EdPinsParent)
				{
					//Take only the output pins
					if (Pin->Direction == EEdGraphPinDirection::EGPD_Output)
					{

						TArray<UEdGraphPin*>& EdPinsChildren = Pin->LinkedTo;
						for (UEdGraphPin* LinkedPin : EdPinsChildren)
						{
							Children.Add(LinkedPin->GetOwningNode());
						}
					}

				}

                EdNode->SaveNodesAsChildren(Children);
			}
			else
			{
				EventFlowSystem_Error_Log("There is no asset node linked to this editor node.");
			}
		}
		else 
		{
			EventFlowSystem_Warning_Log("An unknow EdNode has been found.");
		}
	}

}

void UEventFlowSystemEditorGraph::RefreshNodes()
{
	for (UEdGraphNode* Node : Nodes)
	{
		if (UEventFlowSystemEditorNode* EventFlowEdNode = Cast<UEventFlowSystemEditorNode>(Node))
		{
			EventFlowEdNode->UpdateVisualNode();
		}
	}
}

void UEventFlowSystemEditorGraph::MapNamedNodes()
{
	UEventFlowGraph* Graph = GetGraphAsset();
	Graph->NamedNodes.Empty();
    Graph->NodesNames.Empty();

	for (UEdGraphNode* Node : Nodes)
	{
		if (UEventFlowSystemEditorNode* EventFlowEdNode = Cast<UEventFlowSystemEditorNode>(Node))
		{
			if (EventFlowEdNode->EventFlowBP_Node)
			{
				FName Name = EventFlowEdNode->EventFlowBP_Node->GetFName();
				Graph->NamedNodes.Add(Name.ToString(), EventFlowEdNode->EventFlowBP_Node);
				Graph->NodesNames.Add(EventFlowEdNode->EventFlowBP_Node, Name.ToString());
			}
		}
	}
}

UEventFlowGraph * UEventFlowSystemEditorGraph::GetGraphAsset()
{
	return Cast<UEventFlowGraph>(GetOuter());
}
