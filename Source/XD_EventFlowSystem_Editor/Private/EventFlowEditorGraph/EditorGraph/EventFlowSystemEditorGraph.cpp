// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemEditorGraph.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowSystemEditorNode.h"
#include "EventFlowSystem_Editor_Log.h"
#include "EdGraph/EdGraphPin.h"
#include "EventFlowGraphBlueprint.h"

void UEventFlowSystemEditorGraph::BuildGraph()
{
	LinkAssetNodes();
}

void UEventFlowSystemEditorGraph::ClearOldLinks()
{
	for (UEdGraphNode* EditorNode : Nodes)
	{
		UEventFlowSystemEditorNodeBase* EdNode = Cast<UEventFlowSystemEditorNodeBase>(EditorNode);
		if (EdNode && EdNode->EventFlowBpNode)
		{
			EdNode->EventFlowBpNode->ClearLinks();
		}
	}
}

void UEventFlowSystemEditorGraph::LinkAssetNodes()
{
	ClearOldLinks();
	EventFlowSystem_Log("Starting to link all asset nodes from the editor graph links.");
	for (UEdGraphNode* EditorNode : Nodes)
	{
		if (UEventFlowSystemEditorNodeBase* EdNode = Cast<UEventFlowSystemEditorNodeBase>(EditorNode))
		{
			UEventFlowGraphNodeBase* NodeAsset = EdNode->EventFlowBpNode;
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
		if (UEventFlowSystemEditorNodeBase* EventFlowEdNode = Cast<UEventFlowSystemEditorNodeBase>(Node))
		{
			EventFlowEdNode->UpdateVisualNode();
		}
	}
}

UEventFlowGraphBlueprint* UEventFlowSystemEditorGraph::GetBlueprint() const
{
	return CastChecked<UEventFlowGraphBlueprint>(GetOuter());
}
