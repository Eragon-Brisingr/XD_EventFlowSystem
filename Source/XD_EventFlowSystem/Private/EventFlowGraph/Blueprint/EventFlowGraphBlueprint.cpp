// Fill out your copyright notice in the Description page of Project Settings.

#include "EventFlowGraphBlueprint.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"
#include "XD_EventFlowBase.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowGraph"

#if WITH_EDITOR
FEventFlowDelegateRuntimeBinding FEventFlowDelegateEditorBinding::ToRuntimeBinding(UEventFlowGraphBlueprint* Blueprint) const
{
	FEventFlowDelegateRuntimeBinding RuntimeBinding;
	RuntimeBinding.FunctionName = GetFunctionName(Blueprint);
	RuntimeBinding.PropertyName = PropertyName;
	RuntimeBinding.ObjectName = CastChecked<UEventFlowGraphNodeBase>(Object.Get())->GetVarRefName();
	return RuntimeBinding;
}

FName FEventFlowDelegateEditorBinding::GetFunctionName(UEventFlowGraphBlueprint* Blueprint) const
{
	return Blueprint->GetFieldNameFromClassByGuid<UFunction>(Blueprint->GeneratedClass, MemberFunctionGuid);
}
#endif

UEventFlowGraphBlueprint::UEventFlowGraphBlueprint()
{
	
}

#if WITH_EDITOR
UClass* UEventFlowGraphBlueprint::GetBlueprintClass() const
{
	return UEventFlowGraphBlueprintGeneratedClass::StaticClass();
}

void UEventFlowGraphBlueprint::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	AllowedChildrenOfClasses.Add(UXD_EventFlowBase::StaticClass());
}
#endif

#undef LOCTEXT_NAMESPACE
