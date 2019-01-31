﻿// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EventFlowGraphNodeBase.generated.h"

class UXD_EventFlowBase;

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

/**
 * Base abstract class for every node of the graph.
 */
UCLASS(Blueprintable, BlueprintType)
class XD_EVENTFLOWSYSTEM_API UEventFlowGraphNodeBase : public UObject
{
	GENERATED_BODY()
	
public:
	UEventFlowGraphNodeBase();

    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Node Data")
	FText GetNodeTitle();
    virtual FText GetNodeTitle_Implementation();

	virtual UEventFlowGraphNodeBase* GetDuplicatedNode(UObject* Outer) const;
public:
	UPROPERTY(SaveGame)
	uint8 bIsVariable : 1;

	virtual FString GetVarRefName() const;

	void TryBindRefAndDelegate(UXD_EventFlowBase* EventFlow, bool ForceTry);
};

#undef LOCTEXT_NAMESPACE