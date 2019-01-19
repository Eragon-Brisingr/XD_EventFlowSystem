// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EventFlowGraphBlueprintGeneratedClass.generated.h"

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
	class UEventFlowGraph* EventFlowGraph;

	UPROPERTY()
	TArray<FEventFlowDelegateRuntimeBinding> Bindings;
};
