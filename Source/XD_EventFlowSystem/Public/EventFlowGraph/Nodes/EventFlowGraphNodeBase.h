// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EventFlowGraphNodeBase.generated.h"

class UXD_EventFlowBase;

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

/**
 * Base abstract class for every node of the graph.
 */
UENUM()
enum class EEventFlowCompileMessageType : uint8
{
	None,
	Error,
	Warning
};

UCLASS(abstract)
class XD_EVENTFLOWSYSTEM_API UEventFlowGraphNodeBase : public UObject
{
	GENERATED_BODY()
	
public:
	UEventFlowGraphNodeBase();

    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Node Data")
	FText GetNodeTitle();
    virtual FText GetNodeTitle_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Node Data")
	EEventFlowCompileMessageType GetCompileMessage(FString& Message) const;
	virtual EEventFlowCompileMessageType GetCompileMessage_Implementation(FString& Message) const { return EEventFlowCompileMessageType::None; }

	virtual UEventFlowGraphNodeBase* CreateInstanceByTemplate(UObject* Outer) const;
public:
	UPROPERTY(SaveGame)
	uint8 bIsVariable : 1;

	virtual FString GetVarRefName() const;

protected:
	uint8 bIsBinded : 1;

	void BindRefAndDelegate(UXD_EventFlowBase* EventFlow);
};

#undef LOCTEXT_NAMESPACE