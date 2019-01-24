// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_EventFlowManager.h"
#include "XD_EventFlowSystem_Log.h"
#include "XD_EventFlowBase.h"

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

	DOREPLIFETIME(UXD_EventFlowManager, UnderwayEventFlowList);
	DOREPLIFETIME(UXD_EventFlowManager, FinishEventFlowList);
}

bool UXD_EventFlowManager::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	ReplicatedEventFlowList(UnderwayEventFlowList, WroteSomething, Channel, Bunch, RepFlags);
	ReplicatedEventFlowList(FinishEventFlowList, WroteSomething, Channel, Bunch, RepFlags);

	return WroteSomething;
}

void UXD_EventFlowManager::WhenPostLoad_Implementation()
{
	//恢复游戏事件
	for (UXD_EventFlowBase* EventFlow : UnderwayEventFlowList)
	{
		EventFlow->ReinitEventFlow(this);

		GetWorld()->GetTimerManager().SetTimerForNextTick([=]()
		{
			EventFlow->ReactiveEventFlow();
		});
	}
	for (UXD_EventFlowBase* EventFlow : FinishEventFlowList)
	{
		EventFlow->ReinitEventFlow(this);
	}
}

void UXD_EventFlowManager::ReplicatedEventFlowList(const TArray<UXD_EventFlowBase*>& EventFlowList, bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	for (UXD_EventFlowBase* EventFlow : EventFlowList)
	{
		if (EventFlow)
		{
			WroteSomething |= Channel->ReplicateSubobject(EventFlow, *Bunch, *RepFlags);
			EventFlow->ReplicatedEventFlowSequence(WroteSomething, Channel, Bunch, RepFlags);
		}
	}
}

void UXD_EventFlowManager::OnRep_UnderwayEventFlowList()
{
	for (auto AddEventFlow : TSet<UXD_EventFlowBase*>(UnderwayEventFlowList).Difference(TSet<UXD_EventFlowBase*>(PreUnderwayEventFlowList)))
	{
		if (AddEventFlow)
		{
			AddEventFlow->EventFlowOwner = this;
			OnAcceptEventFlow.Broadcast(AddEventFlow);
		}
	}

	//提示游戏事件完成
	{
		for (auto RemoveEventFlow : TSet<UXD_EventFlowBase*>(PreUnderwayEventFlowList).Difference(TSet<UXD_EventFlowBase*>(UnderwayEventFlowList)))
		{
			if (RemoveEventFlow)
			{
				RemoveEventFlow->WhenFinishEventFlow();
				OnRemoveUnderwayEventFlow.Broadcast(RemoveEventFlow);
			}
		}
	}
	PreUnderwayEventFlowList = UnderwayEventFlowList;
}

void UXD_EventFlowManager::OnRep_FinishEventFlowList()
{
	if (OnFinishEventFlow.IsBound())
	{
		for (auto FinishEventFlow : TSet<UXD_EventFlowBase*>(FinishEventFlowList).Difference(TSet<UXD_EventFlowBase*>(PreFinishEventFlowList)))
		{
			if (FinishEventFlow)
			{
				FinishEventFlow->EventFlowOwner = this;
				OnFinishEventFlow.Broadcast(FinishEventFlow);
			}
		}
	}
	if (OnRemoveFinishEventFlow.IsBound())
	{
		for (auto RemoveEventFlow : TSet<UXD_EventFlowBase*>(PreFinishEventFlowList).Difference(TSet<UXD_EventFlowBase*>(FinishEventFlowList)))
		{
			if (RemoveEventFlow)
			{
				OnRemoveFinishEventFlow.Broadcast(RemoveEventFlow);
			}
		}
	}
	PreFinishEventFlowList = FinishEventFlowList;
}

void UXD_EventFlowManager::ActiveEventFlow(TSubclassOf<UXD_EventFlowBase> EventFlowGraph)
{
	if (GetOwner()->HasAuthority() && EventFlowGraph)
	{
		class UXD_EventFlowBase* EventFlowInstance = NewObject<UXD_EventFlowBase>(this, EventFlowGraph);
		EventFlowSystem_Display_Log("%s 接受了游戏事件 %s", *UXD_DebugFunctionLibrary::GetDebugName(GetOwner()), *EventFlowInstance->GetEventFlowName().ToString());

		UnderwayEventFlowList.Add(EventFlowInstance);
		OnRep_UnderwayEventFlowList();
		//最后再激活，防止游戏事件瞬间完成
		EventFlowInstance->ActiveEventFlow(this);
	}
}

bool UXD_EventFlowManager::IsEventFlowExistInUnderwayList(TSubclassOf<UXD_EventFlowBase> EventFlowGraph)
{
	if (EventFlowGraph)
	{
		for (UXD_EventFlowBase* E_EventFlow : UnderwayEventFlowList)
		{
			if (E_EventFlow->GetClass() == EventFlowGraph)
			{
				return true;
			}
		}
	}
	return false;
}

bool UXD_EventFlowManager::IsEventFlowExistInFinishList(TSubclassOf<UXD_EventFlowBase> EventFlowGraph)
{
	if (EventFlowGraph)
	{
		for (UXD_EventFlowBase* E_EventFlow : FinishEventFlowList)
		{
			if (E_EventFlow->GetClass() == EventFlowGraph)
			{
				return true;
			}
		}
	}
	return false;
}

bool UXD_EventFlowManager::IsEventFlowExist(TSubclassOf<UXD_EventFlowBase> EventFlowGraph)
{
	return IsEventFlowExistInUnderwayList(EventFlowGraph) || IsEventFlowExistInFinishList(EventFlowGraph);
}

