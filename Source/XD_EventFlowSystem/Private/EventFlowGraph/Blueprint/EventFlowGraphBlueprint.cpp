// Fill out your copyright notice in the Description page of Project Settings.

#include "EventFlowGraphBlueprint.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"
#include "EventFlowGraphBase.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowGraph"

UEventFlowGraphBlueprint::UEventFlowGraphBlueprint()
{
	
}

UClass* UEventFlowGraphBlueprint::GetBlueprintClass() const
{
	return UEventFlowGraphBlueprintGeneratedClass::StaticClass();
}

void UEventFlowGraphBlueprint::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	AllowedChildrenOfClasses.Add(UEventFlowGraphBase::StaticClass());
}

#if WITH_EDITOR
TArray<UEventFlowGraphNodeBase*> UEventFlowGraphBlueprint::GetAllNodes() const
{
	if (StartSequence)
	{
		struct FNodeCollector
		{
			static void Collector(UEventFlowGraphNodeBase* Root, TArray<UEventFlowGraphNodeBase*>& Res)
			{
				Res.Add(Root);
				for (UEventFlowGraphNodeBase* Child : Root->GetChildNodes())
				{
					Collector(Child, Res);
				}
			}
		};
		TArray<UEventFlowGraphNodeBase*> Res;
		FNodeCollector::Collector((UEventFlowGraphNodeBase*)StartSequence, Res);
		return Res;
	}
	return {};
}
#endif // WITH_EDITOR

bool FEventFlowDelegateEditorBinding::DoesBindingTargetExist(UEventFlowGraphBlueprint* Blueprint) const
{
	//return Blueprint->EventFlowGraph->GetAllNodes().ContainsByPredicate([this](const UEventFlowGraphNodeBase* Node) {return Node && Node == Object.Get(); });
	return false;
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
