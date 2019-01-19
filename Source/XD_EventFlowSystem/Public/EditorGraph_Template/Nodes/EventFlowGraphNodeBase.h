// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EventFlowGraphNodeBase.generated.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

class UEventFlowGraph;
/**
 * Base abstract class for every node of the graph.
 */
UCLASS(Blueprintable, BlueprintType)
class XD_EVENTFLOWSYSTEM_API UEventFlowGraphNodeBase : public UObject
{
	GENERATED_BODY()
	
public:
	UEventFlowGraphNodeBase();

	UFUNCTION(BlueprintPure, Category = "Node Data")
	TArray<UEventFlowGraphNodeBase*> GetChildren();

	UFUNCTION(BlueprintPure, Category = "Node Data")
	TArray<UEventFlowGraphNodeBase*> GetParents();

    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Node Data")
	FText GetNodeTitle();
    virtual FText GetNodeTitle_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Node Data")
	bool HasInputPins();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Node Data")
	bool HasOutputPins();

	UFUNCTION(BlueprintNativeEvent, Category = "Node Data")
	UEventFlowGraphNodeBase* GetNodePointer();
    virtual UEventFlowGraphNodeBase* GetNodePointer_Implementation();

	virtual void SetGraph(UEventFlowGraph* InGraph);
	virtual UEventFlowGraph* GetGraph();
	virtual void AddToChildren(UEventFlowGraphNodeBase* NewChildNode);
	virtual void AddToParent(UEventFlowGraphNodeBase* NewParentNode);
	virtual bool HasParentNodes();
	virtual void LinkArgumentNodeAsChild(UEventFlowGraphNodeBase* Child);
	virtual void ClearLinks();
	virtual bool RemoveLinkedNode(UEventFlowGraphNodeBase* NodeToRemove);
	virtual bool RemoveNodeFromParents(UEventFlowGraphNodeBase* NodeToRemove);
	virtual bool RemoveNodeFromChilds(UEventFlowGraphNodeBase* NodeToRemove);

	UPROPERTY()
	uint8 bIsVariable : 1;
protected:
	UPROPERTY()
	UEventFlowGraph * Graph = nullptr;
	UPROPERTY()
	TArray<UEventFlowGraphNodeBase*> ParentNodes;
	UPROPERTY()
	TArray<UEventFlowGraphNodeBase*> ChildNodes;
    UPROPERTY()
	bool bHasInputPins = true;
    UPROPERTY()
	bool bHasOutputPins = true;

public:
	UPROPERTY(EditAnywhere, Category = "节点")
	FText Describe;

	DECLARE_DYNAMIC_DELEGATE_RetVal(FText, FDescribeDelegate);
	UPROPERTY()
	FDescribeDelegate DescribeDelegate;
};

#undef LOCTEXT_NAMESPACE