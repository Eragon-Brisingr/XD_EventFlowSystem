﻿// Fill out your copyright notice in the Description page of Project Settings.


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
	void BuildSubobjectMapping(UObject* OtherObject, TMap<UObject*, UObject*>& ObjectMapping) const;

public:
	void RefreshNodes();

	UXD_EventFlowSequenceBase* BuildSequenceTreeInstance(UEventFlowGraphBlueprintGeneratedClass* Outer, FCompilerResultsLog& MessageLog) const;

	UPROPERTY()
	UEventFlowSystemStartEdNode* StartNode;

	//记录任务元素
	UPROPERTY()
	TArray<class UEventElementEdNode*> EventElements;

	UEventFlowGraphBlueprint* GetBlueprint() const;

	TArray<class UEventFlowSystemEditorNodeBase*> GetAllNodes() const;
};
