// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowGraphNodeBase.h"

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


#undef LOCTEXT_NAMESPACE