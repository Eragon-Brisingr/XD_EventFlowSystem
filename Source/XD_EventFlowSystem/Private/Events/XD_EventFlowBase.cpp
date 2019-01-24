// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_EventFlowBase.h"
#include <UnrealNetwork.h>
#include <Engine/ActorChannel.h>
#include "XD_EventFlowManager.h"
#include "XD_EventFlowSequenceBase.h"
#include "XD_DebugFunctionLibrary.h"
#include "XD_EventFlowSystem_Log.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"
#include "XD_ObjectFunctionLibrary.h"

UXD_EventFlowBase::UXD_EventFlowBase()
	:bIsShowEventFlow(true)
{

}

// UXD_EventFlowBase* UXD_EventFlowBase::NewEventFlow(UObject* WorldContextObject, class UXD_EventFlowGraph* EventFlowGraph)
// {
// 	UXD_EventFlowBase* EventFlow = NewObject<UXD_EventFlowBase>(WorldContextObject);
// 	EventFlow->WorldContext = WorldContextObject->GetWorld()->GetGameInstance()->GetWorldContext();
// 	EventFlow->EventFlowTemplate = EventFlowGraph;
// 	return EventFlow;
// }

void UXD_EventFlowBase::ReinitEventFlow(class UXD_EventFlowManager* EventFlowOwner)
{
	for (UXD_EventFlowSequenceBase* EventFlowSequence : CurrentEventFlowSequenceList)
	{
		this->EventFlowOwner = EventFlowOwner;
		WorldContext = EventFlowOwner->GetWorld()->GetGameInstance()->GetWorldContext();
		if (EventFlowSequence)
		{
			EventFlowSequence->OwingEventFlow = this;
			EventFlowSequence->ReinitEventFlowSequence();
		}
	}
}

void UXD_EventFlowBase::ReactiveEventFlow()
{
	GetUnderwayEventFlowSequence()->ActiveEventFlowSequence();
}

FText UXD_EventFlowBase::GetEventFlowName() const
{
	return EventFlowName;
}

#if WITH_EDITOR
void UXD_EventFlowBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif //WITH_EDITOR

class UWorld* UXD_EventFlowBase::GetWorld() const
{
	return EventFlowOwner ? EventFlowOwner->GetWorld() : WorldContext ? WorldContext->World() : nullptr;
}

bool UXD_EventFlowBase::IsSupportedForNetworking() const
{
	return bIsShowEventFlow;
}

void UXD_EventFlowBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UXD_EventFlowBase, CurrentEventFlowSequenceList);
}

void UXD_EventFlowBase::ReplicatedEventFlowSequence(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	for (UXD_EventFlowSequenceBase* EventFlowSequence : CurrentEventFlowSequenceList)
	{
		if (EventFlowSequence)
		{
			WroteSomething |= Channel->ReplicateSubobject(EventFlowSequence, *Bunch, *RepFlags);
			EventFlowSequence->ReplicatedEventFlowElement(WroteSomething, Channel, Bunch, RepFlags);
		}
	}
}

void UXD_EventFlowBase::OnRep_CurrentEventFlowSequenceList()
{
	for (UXD_EventFlowSequenceBase* EventFlowSequence : CurrentEventFlowSequenceList)
	{
		if (EventFlowSequence)
		{
			EventFlowSequence->OwingEventFlow = this;
		}
	}
	if (CurrentEventFlowSequenceList.Num() > 1)
	{
		WhenFinishedEventFlowSequence(CurrentEventFlowSequenceList[CurrentEventFlowSequenceList.Num() - 2], GetUnderwayEventFlowSequence());
	}
}

void UXD_EventFlowBase::SetAndActiveNextEventFlowSequence(class UXD_EventFlowSequenceBase* NextEventFlowSequence)
{
	UXD_EventFlowSequenceBase* FinishEventFlowSequence = GetUnderwayEventFlowSequence();
	EventFlowSystem_Display_Log("%s完成[%s]中的游戏事件序列[%s]", *UXD_DebugFunctionLibrary::GetDebugName(GetEventFlowOwnerCharacter()), *GetEventFlowName().ToString(), *FinishEventFlowSequence->GetDescribe().ToString());
	if (NextEventFlowSequence)
	{
		FinishEventFlowSequence->DeactiveEventFlowSequence();
		CurrentEventFlowSequenceList.Add(NextEventFlowSequence);
		OnRep_CurrentEventFlowSequenceList();
		NextEventFlowSequence->ActiveEventFlowSequence();
		WhenFinishedEventFlowSequence(FinishEventFlowSequence, NextEventFlowSequence);
	}
	else
	{
		WhenFinishedEventFlowSequence(FinishEventFlowSequence, nullptr);
		FinishEventFlowSucceed();
	}
}

class APawn* UXD_EventFlowBase::GetEventFlowOwnerCharacter() const
{
	return GetEventFlowOwnerController()->GetPawn();
}

class AController* UXD_EventFlowBase::GetEventFlowOwnerController() const
{
	if (AController* Controller = Cast<AController>(EventFlowOwner->GetOwner()))
	{
		return Controller;
	}
	else if (APawn* Pawn = Cast<APawn>(EventFlowOwner->GetOwner()))
	{
		return Pawn->GetController();
	}
	return nullptr;
}

void UXD_EventFlowBase::ActiveEventFlow(class UXD_EventFlowManager* EventFlowExecuter)
{
	if (UEventFlowGraphBlueprintGeneratedClass* GeneratedClass = Cast<UEventFlowGraphBlueprintGeneratedClass>(GetClass()))
	{
		if (GeneratedClass->StartSequence)
		{
			EventFlowOwner = EventFlowExecuter;
			EventFlowState = EEventFlowState::Underway;

			CurrentEventFlowSequenceList.Add((UXD_EventFlowSequenceBase*)GeneratedClass->StartSequence->GetDuplicatedNode(this));
			OnRep_CurrentEventFlowSequenceList();
			GetUnderwayEventFlowSequence()->ActiveEventFlowSequence();
		}
		else
		{
			EventFlowSystem_Error_Log("类[%s]中不存在任务序列", *GetClass()->GetName());
		}
	}
	else
	{
		EventFlowSystem_Error_Log("类[%s]的类型不为UEventFlowGraphBlueprintGeneratedClass", *GetClass()->GetName());
	}
}

void FinishEventFlowImpl(UXD_EventFlowBase* EventFlow, UXD_EventFlowManager* EventFlowOwner)
{
	EventFlow->GetUnderwayEventFlowSequence()->DeactiveEventFlowSequence();
	EventFlowOwner->UnderwayEventFlowList.Remove(EventFlow);
	EventFlowOwner->FinishEventFlowList.Add(EventFlow);
	EventFlowOwner->OnRep_UnderwayEventFlowList();
	EventFlowOwner->OnRep_FinishEventFlowList();
}

void UXD_EventFlowBase::FinishEventFlowSucceed()
{
	FinishEventFlowImpl(this, EventFlowOwner);

	EventFlowState = EEventFlowState::Finish_Succeed;
	EventFlowSystem_Display_Log("%s游戏事件[%s]成功完成", *UXD_DebugFunctionLibrary::GetDebugName(GetEventFlowOwnerCharacter()), *GetName());
}

void UXD_EventFlowBase::FinishEventFlowFailed()
{
	FinishEventFlowImpl(this, EventFlowOwner);

	EventFlowState = EEventFlowState::Finish_Succeed;
	EventFlowSystem_Display_Log("%s游戏事件[%s]失败", *UXD_DebugFunctionLibrary::GetDebugName(GetEventFlowOwnerCharacter()), *GetName());
}

void UXD_EventFlowBase::WhenFinishedEventFlowSequence_Implementation(class UXD_EventFlowSequenceBase* FinishedEventFlowSequence, class UXD_EventFlowSequenceBase* UnderwayEventFlowSequences)
{
	EventFlowOwner->OnFinishedEventFlowSequence.Broadcast(this, FinishedEventFlowSequence, UnderwayEventFlowSequences);
}
