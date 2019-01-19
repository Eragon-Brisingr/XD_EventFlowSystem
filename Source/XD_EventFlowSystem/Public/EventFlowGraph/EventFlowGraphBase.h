// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EventFlowGraphBase.generated.h"

class UEventFlowGraph;

/**
 * 
 */
UCLASS(Abstract)
class XD_EVENTFLOWSYSTEM_API UEventFlowGraphBase : public UObject
{
	GENERATED_BODY()
public:
	UEventFlowGraph* GetGraphTemplate() const;
};
