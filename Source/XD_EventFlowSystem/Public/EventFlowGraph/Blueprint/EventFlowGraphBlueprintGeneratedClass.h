// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EventFlowGraphBlueprintGeneratedClass.generated.h"

class UXD_EventFlowSequenceBase;

/**
 * 
 */
USTRUCT()
struct XD_EVENTFLOWSYSTEM_API FEventFlowDelegateRuntimeBinding
{
	GENERATED_BODY()
public:
	/** The widget that will be bound to the live data. */
	UPROPERTY()
	FString ObjectName;

	/** The function or property we're binding to on the source object. */
	UPROPERTY()
	FName FunctionName;
};


UCLASS()
class XD_EVENTFLOWSYSTEM_API UEventFlowGraphBlueprintGeneratedClass : public UBlueprintGeneratedClass
{
	GENERATED_BODY()
public:
	UPROPERTY()
	UXD_EventFlowSequenceBase* StartSequence;

	UPROPERTY()
	TArray<FEventFlowDelegateRuntimeBinding> Bindings;

	UPROPERTY()
	TArray<UXD_EventFlowSequenceBase*> SequenceList;
};
