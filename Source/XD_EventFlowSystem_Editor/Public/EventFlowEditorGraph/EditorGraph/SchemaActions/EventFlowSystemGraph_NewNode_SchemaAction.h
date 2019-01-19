// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "SubclassOf.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowSystemGraph_NewNode_SchemaAction.generated.h"

/**
 * 
 */
USTRUCT()
struct FEventFlowSystemGraph_NewNode_SchemaAction : public FEdGraphSchemaAction
{
public:
	GENERATED_USTRUCT_BODY()

	FEventFlowSystemGraph_NewNode_SchemaAction() :FEdGraphSchemaAction(), NewNodeClass(nullptr) {}
	FEventFlowSystemGraph_NewNode_SchemaAction(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, TSubclassOf<UEventFlowGraphNodeBase> InNodeClass) :FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NewNodeClass(InNodeClass) {}

	/** Execute this action, given the graph and schema, and possibly a pin that we were dragged from. Returns a node that was created by this action (if any). */
	UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;

    virtual UEdGraphNode* CreateEditorNode(UEdGraph * ParentGraph, bool bSelectNewNode, UEventFlowGraphNodeBase* AssetNode);
	TSubclassOf<UEventFlowGraphNodeBase> NewNodeClass;
};
