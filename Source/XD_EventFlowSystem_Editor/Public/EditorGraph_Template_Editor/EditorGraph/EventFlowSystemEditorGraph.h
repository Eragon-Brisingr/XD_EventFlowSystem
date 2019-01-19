﻿// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "EventFlowSystemEditorGraph.generated.h"

/**
 * 
 */
class UEventFlowGraph;

UCLASS()
class UEventFlowSystemEditorGraph : public UEdGraph
{
	GENERATED_BODY()
	
public:
	virtual void BuildGraph();
	virtual void ClearOldLinks();
	virtual void LinkAssetNodes();
	virtual void RefreshNodes();
	virtual void MapNamedNodes();
	virtual UEventFlowGraph* GetGraphAsset();
};
