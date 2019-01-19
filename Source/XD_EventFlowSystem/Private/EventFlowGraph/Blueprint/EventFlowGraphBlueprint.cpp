// Fill out your copyright notice in the Description page of Project Settings.

#include "EventFlowGraphBlueprint.h"
#include "EventFlowGraph.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"
#include "EventFlowGraphBase.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowGraph"

UEventFlowGraphBlueprint::UEventFlowGraphBlueprint()
{
	EventFlowGraph = CreateDefaultSubobject<UEventFlowGraph>(GET_MEMBER_NAME_CHECKED(UEventFlowGraphBlueprint, EventFlowGraph));
}

UClass* UEventFlowGraphBlueprint::GetBlueprintClass() const
{
	return UEventFlowGraphBlueprintGeneratedClass::StaticClass();
}

void UEventFlowGraphBlueprint::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	AllowedChildrenOfClasses.Add(UEventFlowGraphBase::StaticClass());
}

bool FEventFlowDelegateEditorBinding::DoesBindingTargetExist(UEventFlowGraphBlueprint* Blueprint) const
{
	return Blueprint->EventFlowGraph->GetAllNodes().ContainsByPredicate([this](const UEventFlowGraphNodeBase* Node) {return Node && Node == Object.Get(); });
}

FEventFlowDelegateRuntimeBinding FEventFlowDelegateEditorBinding::ToRuntimeBinding(UEventFlowGraphBlueprint* Blueprint) const
{
	FEventFlowDelegateRuntimeBinding RuntimeBinding;
	RuntimeBinding.FunctionName = GetFunctionName(Blueprint);
	RuntimeBinding.ObjectName = Object.Get()->GetName();
	return RuntimeBinding;
}

FName FEventFlowDelegateEditorBinding::GetFunctionName(UEventFlowGraphBlueprint* Blueprint) const
{
	return Blueprint->GetFieldNameFromClassByGuid<UFunction>(Blueprint->GeneratedClass, MemberFunctionGuid);
}

#undef LOCTEXT_NAMESPACE
