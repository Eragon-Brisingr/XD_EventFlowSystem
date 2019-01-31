// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_EventFlowElementBase.h"
#include "XD_DebugFunctionLibrary.h"
#include "TimerManager.h"


#define LOCTEXT_NAMESPACE "游戏事件"

UXD_EventFlowElementBase::UXD_EventFlowElementBase()
	:bIsMust(true), bIsShowEventFlowElement(true), bIsFinished(false)//, bIsActive(true)
{

}

class UWorld* UXD_EventFlowElementBase::GetWorld() const
{
	return OwingEventFlowSequence ? OwingEventFlowSequence->GetWorld() : nullptr;
}

bool UXD_EventFlowElementBase::IsSupportedForNetworking() const
{
	return true;
}

void UXD_EventFlowElementBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UXD_EventFlowElementBase, bIsFinished);
	DOREPLIFETIME(UXD_EventFlowElementBase, bIsMust);
	DOREPLIFETIME(UXD_EventFlowElementBase, bIsShowEventFlowElement);
	DOREPLIFETIME(UXD_EventFlowElementBase, ElementTemplate);
}

FString UXD_EventFlowElementBase::GetVarRefName() const
{
	if (ElementTemplate && OwingEventFlowSequence->SequenceTemplate)
	{
		return FString::Printf(TEXT("Ref_%s_%s"), *OwingEventFlowSequence->SequenceTemplate->GetName(), *ElementTemplate->GetName());
	}
	else
	{
		return FString::Printf(TEXT("Ref_%s_%s"), *OwingEventFlowSequence->GetName(), *GetName());
	}
}

UXD_EventFlowElementBase* UXD_EventFlowElementBase::GetDuplicatedNode(UObject* Outer) const
{
	UXD_EventFlowElementBase* Element = CastChecked<UXD_EventFlowElementBase>(Super::GetDuplicatedNode(Outer));
	Element->ElementTemplate = this;
	return Element;
}

void UXD_EventFlowElementBase::OnRep_ElementTemplate()
{
	if (OwingEventFlowSequence && OwingEventFlowSequence->OwingEventFlow)
	{
		TryBindRefAndDelegate(OwingEventFlowSequence->OwingEventFlow);
	}
}

void UXD_EventFlowElementBase::TryBindRefAndDelegate(UXD_EventFlowBase* EventFlow)
{
	if (ElementTemplate && ElementTemplate->bIsVariable && OwingEventFlowSequence && OwingEventFlowSequence->SequenceTemplate)
	{
		BindRefAndDelegate(EventFlow);
	}
}

FText UXD_EventFlowElementBase::GetDescribe()
{
	return bIsMust ? ReceiveGetDescribe() : FText::Format(LOCTEXT("EventFlowElementFormat", "{0} (可选)"), ReceiveGetDescribe());
}

FText UXD_EventFlowElementBase::ReceiveGetDescribe_Implementation() const
{
	return FText();
}

void UXD_EventFlowElementBase::FinishEventFlowElement(const FName& NextBranchTag)
{
	if (bIsFinished == false)
	{
		bIsFinished = true;
		EventFlowSystem_Display_Log("%s完成[%s]中的游戏事件序列%s中的游戏事件元素%s", *UXD_DebugFunctionLibrary::GetDebugName(GetOwningCharacter()), *GetEventFlow()->GetEventFlowName().ToString(), *UXD_DebugFunctionLibrary::GetDebugName(OwingEventFlowSequence), *UXD_DebugFunctionLibrary::GetDebugName(this));
		OwingEventFlowSequence->InvokeFinishEventFlowSequence(this, NextBranchTag);
	}
}

void UXD_EventFlowElementBase::SetReactive()
{
	if (bIsFinished == true)
	{
		bIsFinished = false;
		if (bIsMust && OwingEventFlowSequence->EventFlowElementList.Contains(this))
		{
			OwingEventFlowSequence->WhenEventFlowElementReactive();
		}
	}
}

void UXD_EventFlowElementBase::ActivateEventFlowElement()
{
#if WITH_EDITOR
	check(bIsActive == false);
	bIsActive = true;
#endif
	EventFlowSystem_Display_Log("%s激活[%s]游戏事件序列%s中的%s", *UXD_DebugFunctionLibrary::GetDebugName(GetOwningCharacter()), *OwingEventFlowSequence->OwingEventFlow->GetEventFlowName().ToString(), *UXD_DebugFunctionLibrary::GetDebugName(OwingEventFlowSequence), *UXD_DebugFunctionLibrary::GetDebugName(this));
	WhenActivateEventFlowElement(GetOwningCharacter(), GetOwingController());
}

void UXD_EventFlowElementBase::UnactiveEventFlowElement()
{
#if WITH_EDITOR
	check(bIsActive);
	bIsActive = false;
#endif
	WhenDeactiveEventFlowElement(GetOwningCharacter(), GetOwingController());
	if (bIsFinished)
	{
		WhenFinishEventFlowElement(GetOwningCharacter(), GetOwingController());
	}
}

class AController* UXD_EventFlowElementBase::GetOwingController() const
{
	return OwingEventFlowSequence->OwingEventFlow->GetEventFlowOwnerController();
}

class APawn* UXD_EventFlowElementBase::GetOwningCharacter() const
{
	return OwingEventFlowSequence->OwingEventFlow->GetEventFlowOwnerCharacter();
}

class UXD_EventFlowBase* UXD_EventFlowElementBase::GetEventFlow() const
{
	return OwingEventFlowSequence ? OwingEventFlowSequence->OwingEventFlow : nullptr;
}

void UElementTest::WhenActivateEventFlowElement_Implementation(class APawn* EventFlowOwnerCharacter, class AController* EventFlowOwner)
{
 	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]() { FinishEventFlowElement(); }), FMath::RandBool() ? 1.f : 2.f, false);
}

void UElementTest::WhenDeactiveEventFlowElement_Implementation(class APawn* EventFlowOwnerCharacter, class AController* EventFlowOwner)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

#undef LOCTEXT_NAMESPACE

