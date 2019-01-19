// Fill out your copyright notice in the Description page of Project Settings.

#include "EventFlowGraphBase.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"

UEventFlowGraph* UEventFlowGraphBase::GetGraphTemplate() const
{
	UEventFlowGraphBlueprintGeneratedClass* BlueprintGeneratedClass = Cast<UEventFlowGraphBlueprintGeneratedClass>(GetClass());
	return BlueprintGeneratedClass->EventFlowGraph;
}
