﻿// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowSystemEditorGraphSchema.generated.h"

/**
 * 
 */
UCLASS()
class UEventFlowSystemEditorGraphSchema : public UEdGraphSchema
{
	GENERATED_UCLASS_BODY()
	
public:
	void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

	void GetContextMenuActions(const UEdGraph* CurrentGraph, const UEdGraphNode* InGraphNode, const UEdGraphPin* InGraphPin, class FMenuBuilder* MenuBuilder, bool bIsDebugging) const override;

	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;

	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;

	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;

};