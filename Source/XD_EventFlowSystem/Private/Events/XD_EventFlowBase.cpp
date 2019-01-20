// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_EventFlowBase.h"
#include <UnrealNetwork.h>
#include <Engine/ActorChannel.h>
#include "XD_EventFlowManager.h"
#include "XD_EventFlowSequenceBase.h"
#include "XD_DebugFunctionLibrary.h"
#include "XD_EventFlowSystem_Log.h"

FEventFlowElementFinishWarpper::FEventFlowElementFinishWarpper(class UXD_EventFlowElementBase* GameEventElement, const TSoftObjectPtr<class UXD_GameEventGraphNode>& GameEventFinishBranch)
	: GameEventElement(GameEventElement)
{

}

FEventFlowElementFinishWarpper::FEventFlowElementFinishWarpper(class UXD_EventFlowElementBase* GameEventElement)
	: GameEventElement(GameEventElement)
{

}

UXD_EventFlowBase::UXD_EventFlowBase()
	:bIsShowGameEvent(true)
{

}

// UXD_EventFlowBase* UXD_EventFlowBase::NewGameEvent(UObject* WorldContextObject, class UXD_GameEventGraph* GameEventGraph)
// {
// 	UXD_EventFlowBase* GameEvent = NewObject<UXD_EventFlowBase>(WorldContextObject);
// 	GameEvent->WorldContext = WorldContextObject->GetWorld()->GetGameInstance()->GetWorldContext();
// 	GameEvent->GameEventTemplate = GameEventGraph;
// 	return GameEvent;
// }

void UXD_EventFlowBase::ReinitGameEvent(class UXD_EventFlowManager* GameEventOwner)
{
	for (UXD_EventFlowSequenceBase* GameEventSequence : CurrentGameEventSequenceList)
	{
		this->GameEventOwner = GameEventOwner;
		WorldContext = GameEventOwner->GetWorld()->GetGameInstance()->GetWorldContext();
		if (GameEventSequence)
		{
			GameEventSequence->OwingGameEvent = this;
			GameEventSequence->ReinitGameEventSequence();
		}
	}
}

void UXD_EventFlowBase::ReactiveGameEvent()
{
	GetUnderwayGameEventSequence()->ActiveGameEventSequence();
}

const UEventFlowGraph* UXD_EventFlowBase::GetEventFlowGraphTemplate() const
{
	return nullptr;
}

FText UXD_EventFlowBase::GetGameEventName() const
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
	return GameEventOwner ? GameEventOwner->GetWorld() : WorldContext ? WorldContext->World() : nullptr;
}

bool UXD_EventFlowBase::IsSupportedForNetworking() const
{
	return bIsShowGameEvent;
}

void UXD_EventFlowBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UXD_EventFlowBase, CurrentGameEventSequenceList);
}

void UXD_EventFlowBase::ReplicatedGameEventSequence(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	for (UXD_EventFlowSequenceBase* GameEventSequence : CurrentGameEventSequenceList)
	{
		if (GameEventSequence)
		{
			WroteSomething |= Channel->ReplicateSubobject(GameEventSequence, *Bunch, *RepFlags);
			GameEventSequence->ReplicatedGameEventElement(WroteSomething, Channel, Bunch, RepFlags);
		}
	}
}

void UXD_EventFlowBase::OnRep_CurrentGameEventSequenceList()
{
	for (UXD_EventFlowSequenceBase* GameEventSequence : CurrentGameEventSequenceList)
	{
		if (GameEventSequence)
		{
			GameEventSequence->OwingGameEvent = this;
		}
	}
	if (CurrentGameEventSequenceList.Num() > 1)
	{
		WhenFinishedGameEventSequence(CurrentGameEventSequenceList[CurrentGameEventSequenceList.Num() - 2], GetUnderwayGameEventSequence());
	}
}

void UXD_EventFlowBase::SetAndActiveNextGameEventSequence(class UXD_EventFlowSequenceBase* NextGameEventSequence)
{
	UXD_EventFlowSequenceBase* FinishGameEventSequence = GetUnderwayGameEventSequence();
	EventFlowSystem_Display_LOG("%s完成[%s]中的游戏事件序列[%s]", *UXD_DebugFunctionLibrary::GetDebugName(GetGameEventOwnerCharacter()), *GetGameEventName().ToString(), *FinishGameEventSequence->GetDescribe().ToString());
	if (NextGameEventSequence)
	{
		FinishGameEventSequence->DeactiveGameEventSequence();
		CurrentGameEventSequenceList.Add(NextGameEventSequence);
		NextGameEventSequence->ActiveGameEventSequence();
		WhenFinishedGameEventSequence(FinishGameEventSequence, NextGameEventSequence);
	}
	else
	{
		WhenFinishedGameEventSequence(FinishGameEventSequence, nullptr);
		FinishGameEvent();
	}
}

class APawn* UXD_EventFlowBase::GetGameEventOwnerCharacter() const
{
	return GeGameEventOwnerController()->GetPawn();
}

class AController* UXD_EventFlowBase::GeGameEventOwnerController() const
{
	if (AController* Controller = Cast<AController>(GameEventOwner->GetOwner()))
	{
		return Controller;
	}
	else if (APawn* Pawn = Cast<APawn>(GameEventOwner->GetOwner()))
	{
		return Pawn->GetController();
	}
	return nullptr;
}

void UXD_EventFlowBase::ActiveGameEvent(class UXD_EventFlowManager* GameEventExecuter)
{
	GameEventOwner = GameEventExecuter;
	GameEventState = EEventFlowState::Underway;

	//CurrentGameEventSequenceList.Add(GameEventTemplate->FirstGameEventGraphNode->GetGameEventSequence(this, nullptr));

	CurrentGameEventSequenceList[0]->ActiveGameEventSequence();
}

void UXD_EventFlowBase::FinishGameEvent()
{
	GetUnderwayGameEventSequence()->DeactiveGameEventSequence();
	GameEventState = EEventFlowState::Finish_Succeed;
	GameEventOwner->UnderwayGameEventList.Remove(this);
	GameEventOwner->FinishGameEventList.Add(this);
	GameEventOwner->OnRep_UnderwayGameEventList();
	GameEventOwner->OnRep_FinishGameEventList();

	EventFlowSystem_Display_LOG("%s完成游戏事件[%s]", *UXD_DebugFunctionLibrary::GetDebugName(GetGameEventOwnerCharacter()), *GetName());
}

void UXD_EventFlowBase::WhenFinishedGameEventSequence_Implementation(class UXD_EventFlowSequenceBase* FinishedGameEventSequence, class UXD_EventFlowSequenceBase* UnderwayGameEventSequences)
{
	GameEventOwner->OnFinishedGameEventSequence.Broadcast(this, FinishedGameEventSequence, UnderwayGameEventSequences);
}

void UXD_EventFlowBase::ForceFinishGameEvent(EEventFlowState State)
{
	FinishGameEvent();
	GameEventState = State;
}
