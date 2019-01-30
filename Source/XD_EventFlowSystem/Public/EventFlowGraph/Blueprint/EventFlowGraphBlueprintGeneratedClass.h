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
	UPROPERTY()
	FString ObjectName;

	UPROPERTY()
	FName PropertyName;

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
