// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowEditorGraph/EditorGraph/EventFlowSystemEditorGraphSchema.h"
#include <Templates/SubclassOf.h>
#include <UObject/UObjectIterator.h>
#include <EdGraph/EdGraph.h>
#include <Modules/ModuleManager.h>

#include "XD_EventFlowSystem_Editor.h"
#include "EventFlowEditorGraph/Utility/EventFlowSystem_Editor_Log.h"
#include "EventFlowEditorGraph/EditorGraph/EventFlowSystemEditorConnectionDrawingPolicy.h"
#include "EventFlowEditorGraph/EditorGraph/SchemaActions/EventFlowSystemGraph_NewNode_SchemaAction.h"
#include "EventFlowEditorGraph/EditorGraph/EditorNodes/EventFlowSystemEditorNode.h"
#include "EventFlowEditorGraph/Utility/EventFlowSystem_Editor_ClassHelper.h"
#include "EventFlowEditorGraph/EditorGraph/EventFlowSystemEditorGraph.h"
#include "Events/XD_EventFlowSequenceBase.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEventFlowSystemEditorGraphSchema::UEventFlowSystemEditorGraphSchema(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

void UEventFlowSystemEditorGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	UEventFlowSystemEditorNodeBase* FromNode = ContextMenuBuilder.FromPin ? CastChecked<UEventFlowSystemEditorNodeBase>(ContextMenuBuilder.FromPin->GetOwningNode()) : nullptr;
	bool overrideGraphContext = FromNode && FromNode->GetNodeLinkableContextActions(ContextMenuBuilder);
	if (!overrideGraphContext)
	{
		FCategorizedGraphActionListBuilder BaseBuilder(TEXT("任务序列"));

		FXD_EventFlowSystem_EditorModule& Module = FModuleManager::LoadModuleChecked<FXD_EventFlowSystem_EditorModule>("XD_EventFlowSystem_Editor");
		TSharedPtr<FEventFlowSystem_Editor_ClassHelper> Helper = Module.GetHelper();
		TArray<FEventFlowSystem_Editor_ClassData> AllSubClasses;
		Helper->GatherClasses(UXD_EventFlowSequenceBase::StaticClass(), AllSubClasses);

		FText ToolTip = LOCTEXT("NewEditorGraphNodeTooltip", "Add a {NodeName} to the graph.");
		FText MenuDesc = LOCTEXT("NewEditorGraphNodeDescription", "{NodeName}");
		for (auto& ClassData : AllSubClasses)
		{
			if (!ClassData.GetClass()->HasAnyClassFlags(CLASS_Abstract))
			{
				FFormatNamedArguments Arguments;
				Arguments.Add(TEXT("NodeName"), ClassData.GetClass()->GetDisplayNameText());
				TSharedPtr<FNewSequence_SchemaAction> NewNodeAction;

				NewNodeAction = MakeShareable(new FNewSequence_SchemaAction(ClassData.GetCategory(), FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments), 0, ClassData.GetClass()));

				BaseBuilder.AddAction(NewNodeAction);
			}
		}

		ContextMenuBuilder.Append(BaseBuilder);
	}
}

const FPinConnectionResponse UEventFlowSystemEditorGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (!(A && B))
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both pins must be available."));

	const UEventFlowSystemEditorNodeBase* NodeA = CastChecked<const UEventFlowSystemEditorNodeBase>(A->GetOwningNode());
	const UEventFlowSystemEditorNodeBase* NodeB = CastChecked<const UEventFlowSystemEditorNodeBase>(B->GetOwningNode());

	if (A->GetOwningNode() == B->GetOwningNode())
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect a node to itself."));

	if (A->Direction == EGPD_Input && B->Direction == EGPD_Input)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect an input pin to another input pin."));

	if (A->Direction == EGPD_Output && B->Direction == EGPD_Output)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect an output pin to another output pin"));

	if (A->Direction == EGPD_Output && B->Direction == EGPD_Input)
	{
		return NodeA->CanLinkedTo(NodeB);
	}
	else if (B->Direction == EGPD_Output && A->Direction == EGPD_Input)
	{
		return NodeB->CanLinkedTo(NodeA);
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

FConnectionDrawingPolicy* UEventFlowSystemEditorGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect & InClippingRect, FSlateWindowElementList & InDrawElements, UEdGraph * InGraphObj) const
{
	return new FEventFlowSystemEditorConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

void UEventFlowSystemEditorGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	UEventFlowSystemEditorGraph* EventFlowEditorGraph = Cast<UEventFlowSystemEditorGraph>(&Graph);

	if (EventFlowEditorGraph && Graph.Nodes.Num() == 0)
	{
		Graph.Modify();

		FGraphNodeCreator<UEventFlowSystemStartEdNode> Creator(Graph);
		UEventFlowSystemStartEdNode* StartNode = Creator.CreateNode();
		EventFlowEditorGraph->StartNode = StartNode;
		StartNode->AllocateDefaultPins();

		Creator.Finalize();

		StartNode->NodePosX = 0;
		StartNode->NodePosY = 0;
	}
}
#undef LOCTEXT_NAMESPACE