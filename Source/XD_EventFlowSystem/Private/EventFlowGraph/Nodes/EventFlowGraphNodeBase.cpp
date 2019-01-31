// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowGraphNodeBase.h"
#include "XD_ObjectFunctionLibrary.h"
#include "XD_EventFlowBase.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"

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

void UEventFlowGraphNodeBase::BindRefAndDelegate(UXD_EventFlowBase* EventFlow)
{
	if (bIsBinded)
	{
		return;
	}
	bIsBinded = true;

	if (UEventFlowGraphBlueprintGeneratedClass* GeneratedClass = Cast<UEventFlowGraphBlueprintGeneratedClass>(EventFlow->GetClass()))
	{
		FString VarRefName = GetVarRefName();
		UProperty* RefProperty = EventFlow->GetClass()->FindPropertyByName(*VarRefName);
		*RefProperty->ContainerPtrToValuePtr<UObject*>(EventFlow) = this;

		for (const FEventFlowDelegateRuntimeBinding& Binding : GeneratedClass->Bindings)
		{
			UDelegateProperty* DelegateProperty = FindField<UDelegateProperty>(GetClass(), *(Binding.PropertyName.ToString() + TEXT("Delegate")));
			if (Binding.ObjectName == GetVarRefName())
			{
				FScriptDelegate* ScriptDelegate = DelegateProperty->GetPropertyValuePtr_InContainer(this);
				ScriptDelegate->BindUFunction(EventFlow, Binding.FunctionName);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE