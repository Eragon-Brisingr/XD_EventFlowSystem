// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SubclassOf.h"
#include "EventFlowGraph.generated.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

class UEventFlowGraphNodeBase;
class UStartNode;

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class XD_EVENTFLOWSYSTEM_API UEventFlowGraph : public UObject
{
	GENERATED_BODY()
	
public:
	UEventFlowGraph();

	UPROPERTY(BlueprintReadOnly, Category = "Graph Data")
	TArray<UEventFlowGraphNodeBase*> Nodes;

	UPROPERTY(BlueprintReadOnly, Category = "Graph Data")
	TMap<FString, UEventFlowGraphNodeBase*> NamedNodes;

    UPROPERTY(BlueprintReadOnly, Category = "Graph Data")
    TMap<UEventFlowGraphNodeBase*, FString> NodesNames;

    UPROPERTY(BlueprintReadWrite, Category = "Graph Data")
    UObject* Owner;

    UFUNCTION(BlueprintCallable, Category = "Graph Data")
    void InitGraph(UObject* ParentObject);

	TArray<UEventFlowGraphNodeBase*> GetAllNodes() const { return Nodes; }

#if WITH_EDITORONLY_DATA

public:

	UPROPERTY()
	class UEdGraph* EdGraph;

	template <class T>
	T* SpawnNodeInsideGraph(TSubclassOf<UEventFlowGraphNodeBase> InNodeClass)
	{
		T* SpawenNode = NewObject<T>(this, InNodeClass, NAME_None, RF_Transactional);
		AddNode(SpawenNode);
		return SpawenNode;
	}


	virtual void AddNode(UEventFlowGraphNodeBase* InNode);
	virtual void RemoveNode(UEventFlowGraphNodeBase* NodeToRemove);

#endif
};

#undef LOCTEXT_NAMESPACE