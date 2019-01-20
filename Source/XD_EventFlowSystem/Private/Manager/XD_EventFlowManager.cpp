// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_EventFlowManager.h"

// Sets default values for this component's properties
UXD_EventFlowManager::UXD_EventFlowManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UXD_EventFlowManager::BeginPlay()
{
	Super::BeginPlay();

	// ...

	SetIsReplicated(true);
}


// Called every frame
void UXD_EventFlowManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UXD_EventFlowManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UXD_EventFlowManager, UnderwayGameEventList);
	DOREPLIFETIME(UXD_EventFlowManager, FinishGameEventList);
}

bool UXD_EventFlowManager::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	ReplicatedGameEventList(UnderwayGameEventList, WroteSomething, Channel, Bunch, RepFlags);
	ReplicatedGameEventList(FinishGameEventList, WroteSomething, Channel, Bunch, RepFlags);

	return WroteSomething;
}

void UXD_EventFlowManager::WhenPostLoad_Implementation()
{
	//恢复游戏事件
	for (UXD_EventFlowBase* GameEvent : UnderwayGameEventList)
	{
		GameEvent->ReinitGameEvent(this);

		GetWorld()->GetTimerManager().SetTimerForNextTick([=]()
		{
			GameEvent->ReactiveGameEvent();
		});
	}
	for (UXD_EventFlowBase* GameEvent : FinishGameEventList)
	{
		GameEvent->ReinitGameEvent(this);
	}
}

void UXD_EventFlowManager::ReplicatedGameEventList(const TArray<UXD_EventFlowBase*>& GameEventList, bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	for (UXD_EventFlowBase* GameEvent : GameEventList)
	{
		if (GameEvent)
		{
			WroteSomething |= Channel->ReplicateSubobject(GameEvent, *Bunch, *RepFlags);
			GameEvent->ReplicatedGameEventSequence(WroteSomething, Channel, Bunch, RepFlags);
		}
	}
}

void UXD_EventFlowManager::OnRep_UnderwayGameEventList()
{
	for (auto AddGameEvent : TSet<UXD_EventFlowBase*>(UnderwayGameEventList).Difference(TSet<UXD_EventFlowBase*>(PreUnderwayGameEventList)))
	{
		if (AddGameEvent)
		{
			AddGameEvent->GameEventOwner = this;
			OnAcceptGameEvent.Broadcast(AddGameEvent);
		}
	}

	//提示游戏事件完成
	{
		for (auto RemoveGameEvent : TSet<UXD_EventFlowBase*>(PreUnderwayGameEventList).Difference(TSet<UXD_EventFlowBase*>(UnderwayGameEventList)))
		{
			if (RemoveGameEvent)
			{
				RemoveGameEvent->WhenFinishGameEvent();
				OnRemoveUnderwayGameEvent.Broadcast(RemoveGameEvent);
			}
		}
	}
	PreUnderwayGameEventList = UnderwayGameEventList;
}

void UXD_EventFlowManager::OnRep_FinishGameEventList()
{
	if (OnFinishGameEvent.IsBound())
	{
		for (auto FinishGameEvent : TSet<UXD_EventFlowBase*>(FinishGameEventList).Difference(TSet<UXD_EventFlowBase*>(PreFinishGameEventList)))
		{
			if (FinishGameEvent)
			{
				FinishGameEvent->GameEventOwner = this;
				OnFinishGameEvent.Broadcast(FinishGameEvent);
			}
		}
	}
	if (OnRemoveFinishGameEvent.IsBound())
	{
		for (auto RemoveGameEvent : TSet<UXD_EventFlowBase*>(PreFinishGameEventList).Difference(TSet<UXD_EventFlowBase*>(FinishGameEventList)))
		{
			if (RemoveGameEvent)
			{
				OnRemoveFinishGameEvent.Broadcast(RemoveGameEvent);
			}
		}
	}
	PreFinishGameEventList = FinishGameEventList;
}

// void UXD_EventFlowManager::ApplyGameEvent(class UXD_GameEventGraph* GameEventGraph)
// {
// 	if (GetOwner()->HasAuthority() && GameEventGraph)
// 	{
// 		class UXD_EventFlowBase* GameEvent = UXD_EventFlowBase::NewGameEvent(this, GameEventGraph);
// 		EventFlowSystem_Display_LOG("%s 接受了游戏事件 %s", *UXD_DebugFunctionLibrary::GetDebugName(GetOwner()), *GameEvent->GetGameEventName().ToString());
// 
// 		UnderwayGameEventList.Add(GameEvent);
// 		OnRep_UnderwayGameEventList();
// 		//最后再激活，防止游戏事件瞬间完成
// 		GameEvent->ActiveGameEvent(this);
// 	}
// }
// 
// bool UXD_EventFlowManager::IsGameEventExistInUnderwayList(class UXD_GameEventGraph* GameEvent)
// {
// 	if (GameEvent)
// 	{
// 		for (UXD_EventFlowBase* E_GameEvent : UnderwayGameEventList)
// 		{
// 			if (E_GameEvent->IsEqualWithOtherGameEvent(GameEvent))
// 			{
// 				return true;
// 			}
// 		}
// 	}
// 	return false;
// }
// 
// bool UXD_EventFlowManager::IsGameEventExistInFinishList(class UXD_GameEventGraph* GameEvent)
// {
// 	if (GameEvent)
// 	{
// 		for (UXD_EventFlowBase* E_GameEvent : FinishGameEventList)
// 		{
// 			if (E_GameEvent->IsEqualWithOtherGameEvent(GameEvent))
// 			{
// 				return true;
// 			}
// 		}
// 	}
// 	return false;
// }
// 
// bool UXD_EventFlowManager::IsGameEventExist(class UXD_GameEventGraph* GameEvent)
// {
// 	return IsGameEventExistInUnderwayList(GameEvent) || IsGameEventExistInFinishList(GameEvent);
// }


