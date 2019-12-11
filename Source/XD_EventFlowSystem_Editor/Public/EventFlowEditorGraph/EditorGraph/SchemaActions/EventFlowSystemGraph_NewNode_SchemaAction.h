// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include <Templates/SubclassOf.h>
#include "EventFlowGraph/Nodes/EventFlowGraphNodeBase.h"
#include "EventFlowSystemGraph_NewNode_SchemaAction.generated.h"

class UXD_EventFlowSequenceBase;

/**
 * 
 */
USTRUCT()
struct FNewSequence_SchemaAction : public FEdGraphSchemaAction
{
public:
	GENERATED_USTRUCT_BODY()

	FNewSequence_SchemaAction() :FEdGraphSchemaAction(), NewSequenceClass(nullptr) {}
	FNewSequence_SchemaAction(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, TSubclassOf<UXD_EventFlowSequenceBase> NewSequenceClass) :FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NewSequenceClass(NewSequenceClass) {}

	/** Execute this action, given the graph and schema, and possibly a pin that we were dragged from. Returns a node that was created by this action (if any). */
	UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;

	TSubclassOf<UXD_EventFlowSequenceBase> NewSequenceClass;
};
