// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_EventFlowElementBase.h"
#include "XD_DebugFunctionLibrary.h"


#define LOCTEXT_NAMESPACE "游戏事件"

UXD_EventFlowElementBase::UXD_EventFlowElementBase()
	:bIsMust(true), bIsShowGameEventElement(true), bIsFinished(false)//, bIsActive(true)
{

}

class UWorld* UXD_EventFlowElementBase::GetWorld() const
{
	return GetEventFlow() ? GetEventFlow()->GetWorld() : nullptr;
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
	DOREPLIFETIME(UXD_EventFlowElementBase, bIsShowGameEventElement);

}

FText UXD_EventFlowElementBase::GetDescribe()
{
	return bIsMust ? ReceiveGetDescribe() : FText::Format(LOCTEXT("GameEventElementFormat", "{0} (可选)"), ReceiveGetDescribe());
}

FText UXD_EventFlowElementBase::ReceiveGetDescribe_Implementation() const
{
	return FText();
}

void UXD_EventFlowElementBase::FinishGameEventElement(int32 Index)
{
	if (bIsFinished == false)
	{
		bIsFinished = true;
		EventFlowSystem_Display_LOG("%s完成[%s]中的游戏事件序列[%s]中的游戏事件元素[%s]", *UXD_DebugFunctionLibrary::GetDebugName(GetOwningCharacter()), *GetEventFlow()->GetGameEventName().ToString(), *OwingEventFlowSequence->GetDescribe().ToString(), *GetName());
		OwingEventFlowSequence->InvokeFinishGameEventSequence(this, Index);
	}
}

void UXD_EventFlowElementBase::SetReactive()
{
	if (bIsFinished == true)
	{
		bIsFinished = false;
		if (bIsMust && OwingEventFlowSequence->GameEventElementList.Contains(this))
		{
			OwingEventFlowSequence->WhenGameEventElementReactive();
		}
	}
}

void UXD_EventFlowElementBase::ActivateGameEventElement()
{
	WhenActivateGameEventElement(GetOwningCharacter(), GetOwingController());
}

void UXD_EventFlowElementBase::UnactiveGameEventElement()
{
	WhenUnactiveGameEventElement(GetOwningCharacter(), GetOwingController());
	if (bIsFinished)
	{
		WhenFinishGameEventElement(GetOwningCharacter(), GetOwingController());
	}
}

class AController* UXD_EventFlowElementBase::GetOwingController() const
{
	return OwingEventFlowSequence->OwingEventFlow->GeGameEventOwnerController();
}

class APawn* UXD_EventFlowElementBase::GetOwningCharacter() const
{
	return OwingEventFlowSequence->OwingEventFlow->GetGameEventOwnerCharacter();
}

class UXD_EventFlowBase* UXD_EventFlowElementBase::GetEventFlow() const
{
	return OwingEventFlowSequence ? OwingEventFlowSequence->OwingEventFlow : nullptr;
}

#undef LOCTEXT_NAMESPACE

