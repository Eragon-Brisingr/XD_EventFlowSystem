// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "EventFlowSystemEditorGraph.generated.h"

/**
 * 
 */
class UEventFlowSystemStartEdNode;
class UEventFlowGraphBlueprint;
class FCompilerResultsLog;
class UXD_EventFlowSequenceBase;
class UEventFlowGraphBlueprintGeneratedClass;

UCLASS()
class UEventFlowSystemEditorGraph : public UEdGraph
{
	GENERATED_BODY()
	
public:
	virtual void RefreshNodes();

	UXD_EventFlowSequenceBase* BuildSequenceTreeInstance(UEventFlowGraphBlueprintGeneratedClass* Outer, FCompilerResultsLog& MessageLog) const;

	UPROPERTY()
	UEventFlowSystemStartEdNode* StartNode;

	//必须将元素在这边记录下，否则编辑器启动读取时有问题
	UPROPERTY()
	TArray<class UEventElementEdNode*> EventElements;

	UEventFlowGraphBlueprint* GetBlueprint() const;

	TArray<class UEventFlowSystemEditorNodeBase*> GetAllNodes() const;
};
