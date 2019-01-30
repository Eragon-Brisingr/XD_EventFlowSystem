// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowGraphNodeBase.h"
#include "XD_ObjectFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEventFlowGraphNodeBase::UEventFlowGraphNodeBase()
{
	
}

FText UEventFlowGraphNodeBase::GetNodeTitle_Implementation()
{
#if WITH_EDITOR
	return GetClass()->GetDisplayNameText();
#else
	return FText::GetEmpty();
#endif
}

UEventFlowGraphNodeBase* UEventFlowGraphNodeBase::GetDuplicatedNode(UObject* Outer) const
{
	return UXD_ObjectFunctionLibrary::DuplicateObject(this, Outer, GetFName());
}

FString UEventFlowGraphNodeBase::GetVarRefName() const
{
	return FString::Printf(TEXT("Ref_%s"), *GetName());
}

#undef LOCTEXT_NAMESPACE