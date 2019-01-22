// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_EventFlowSequenceBase.h"
#include "XD_EventFlowBase.h"
#include "XD_EventFlowSystem_Log.h"
#include "XD_ObjectFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "游戏事件"

class UWorld* UXD_EventFlowSequenceBase::GetWorld() const
{
	return OwingEventFlow ? OwingEventFlow->GetWorld() : nullptr;
}

bool UXD_EventFlowSequenceBase::IsSupportedForNetworking() const
{
	return true;
}

void UXD_EventFlowSequenceBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UXD_EventFlowSequenceBase, GameEventElementList);
}

void UXD_EventFlowSequenceBase::ReplicatedGameEventElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	for (UXD_EventFlowElementBase* GameEventElement : GameEventElementList)
	{
		if (GameEventElement)
		{
			WroteSomething |= Channel->ReplicateSubobject(GameEventElement, *Bunch, *RepFlags);
		}
	}
}

void UXD_EventFlowSequenceBase::ActiveGameEventSequence()
{
	EventFlowSystem_Display_LOG("%s激活[%s]中的游戏事件序列[%s]", *UXD_DebugFunctionLibrary::GetDebugName(GetGameEventOwnerCharacter()), *OwingEventFlow->GetGameEventName().ToString(), *GetDescribe().ToString());
	for (UXD_EventFlowElementBase* GameEventElement : GameEventElementList)
	{
		GameEventElement->ActivateGameEventElement();
	}
}

bool UXD_EventFlowSequenceBase::HasMustGameEventElement()
{
	for (auto GameEventElement : GameEventElementList)
	{
		if (GameEventElement->bIsMust)
		{
			return true;
		}
	}
	return false;
}

void UXD_EventFlowSequenceBase::DeactiveGameEventSequence()
{
	for (UXD_EventFlowElementBase* GameEventElement : GameEventElementList)
	{
		GameEventElement->UnactiveGameEventElement();
	}
}

void UXD_EventFlowSequenceBase::InvokeFinishGameEventSequence(class UXD_EventFlowElementBase* GameEventElement, int32 Index)
{
	unimplemented();
}

bool UXD_EventFlowSequenceBase::IsEveryMustGameEventElementFinished() const
{
	for (UXD_EventFlowElementBase* GameEventElement : GameEventElementList)
	{
		if (GameEventElement->bIsMust && !GameEventElement->IsFinished())
		{
			return false;
		}
	}
	return true;
}

void UXD_EventFlowSequenceBase::DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch)
{
	for (UXD_EventFlowElementBase* GameEventElement : GameEventElementList)
	{
		GameEventElement->DrawHintInWorld(ARPG_HUD, Index, false);
	}
}

void UXD_EventFlowSequenceBase::AddGameEventElement(class UXD_EventFlowElementBase* GameEventElement)
{
	GameEventElementList.Add(GameEventElement);
}

void UXD_EventFlowSequenceBase::ReinitGameEventSequence()
{
	for (UXD_EventFlowElementBase* GameEventElement : GameEventElementList)
	{
		if (GameEventElement)
		{
			GameEventElement->OwingEventFlowSequence = this;
		}
	}
}

FText UXD_EventFlowSequenceBase::GetDescribe() const
{
	return DescribeDelegate.IsBound() ? DescribeDelegate.Execute() : Describe;
}

void UXD_EventFlowSequenceBase::OnRep_GameEventElementList()
{
	for (UXD_EventFlowElementBase* GameEventElement : GameEventElementList)
	{
		if (GameEventElement)
		{
			GameEventElement->OwingEventFlowSequence = this;
		}
	}
}

class APawn* UXD_EventFlowSequenceBase::GetGameEventOwnerCharacter() const
{
	return OwingEventFlow->GetGameEventOwnerCharacter();
}

void UEventFlowSequence_Branch::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEventFlowSequence_Branch, GameEventElementFinishList);

}

void UEventFlowSequence_Branch::ReplicatedGameEventElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	Super::ReplicatedGameEventElement(WroteSomething, Channel, Bunch, RepFlags);
	for (const FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
	{
		if (GameEventElementFinishWarpper.EventFlowElement)
		{
			WroteSomething |= Channel->ReplicateSubobject(GameEventElementFinishWarpper.EventFlowElement, *Bunch, *RepFlags);
		}
	}
}

void UEventFlowSequence_Branch::ReinitGameEventSequence()
{
	Super::ReinitGameEventSequence();
	for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
	{
		if (GameEventElementFinishWarpper.EventFlowElement)
		{
			GameEventElementFinishWarpper.EventFlowElement->OwingEventFlowSequence = this;
		}
	}
}

void UEventFlowSequence_Branch::ActiveGameEventSequence()
{
	Super::ActiveGameEventSequence();
	InvokeActiveFinishList();
}

void UEventFlowSequence_Branch::DeactiveGameEventSequence()
{
	Super::DeactiveGameEventSequence();
	DeactiveFinishBranchs();
}

void UEventFlowSequence_Branch::InvokeFinishGameEventSequence(class UXD_EventFlowElementBase* GameEventElement, int32 Index)
{
	//激活结束游戏事件的关键游戏事件
	if (GameEventElementList.Contains(GameEventElement))
	{
		InvokeActiveFinishList();
	}
	//结束游戏事件
	else
	{
		UXD_EventFlowSequenceBase* FinishGameEventSequence = OwingEventFlow->GetUnderwayGameEventSequence();
		for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
		{
			if (GameEventElement == GameEventElementFinishWarpper.EventFlowElement)
			{
// 				if (GameEventElementFinishWarpper.GameEventFinishBranch.IsValid())
// 				{
// 					if (GameEventElementFinishWarpper.GameEventFinishBranch->ChildrenNodes.Num() > 0)
// 					{
// 						OwingGameEvent->SetAndActiveNextGameEventSequence(GameEventElementFinishWarpper.GameEventFinishBranch->GetGameEventSequence(OwingGameEvent, NextEdge));
// 					}
// 					else
// 					{
// 						OwingGameEvent->SetAndActiveNextGameEventSequence(nullptr);
// 					}
// 				}
				break;
			}
		}
	}
}

void UEventFlowSequence_Branch::WhenGameEventElementReactive()
{
	DeactiveFinishBranchs();
}

void UEventFlowSequence_Branch::DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch)
{
	Super::DrawHintInWorld(ARPG_HUD, Index, IsFinishBranch);
	if (bIsFinishListActive)
	{
		for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
		{
			GameEventElementFinishWarpper.EventFlowElement->DrawHintInWorld(ARPG_HUD, Index, true);
		}
	}
}

void UEventFlowSequence_Branch::InvokeActiveFinishList()
{
	if (IsEveryMustGameEventElementFinished())
	{
		if (GameEventElementFinishList.Num() == 0)
		{
			OwingEventFlow->FinishGameEvent();
		}
		else
		{
			if (bIsFinishListActive == false)
			{
				for (FEventFlowElementFinishWarpper& GameEventElementFinish : GameEventElementFinishList)
				{
					GameEventElementFinish.EventFlowElement->ActivateGameEventElement();
				}
				bIsFinishListActive = true;
			}
		}
	}
}

void UEventFlowSequence_Branch::DeactiveFinishBranchs()
{
	bIsFinishListActive = false;
	for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
	{
		GameEventElementFinishWarpper.EventFlowElement->UnactiveGameEventElement();
	}
}

void UEventFlowSequence_Branch::OnRep_GameEventElementFinishList()
{
	for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
	{
		if (GameEventElementFinishWarpper.EventFlowElement)
		{
			GameEventElementFinishWarpper.EventFlowElement->OwingEventFlowSequence = this;
		}
	}
}

void UEventFlowSequence_List::InvokeFinishGameEventSequence(class UXD_EventFlowElementBase* GameEventElement, int32 Index)
{
	if (IsEveryMustGameEventElementFinished())
	{
		if (NextSequence.IsValid())
		{
			OwingEventFlow->SetAndActiveNextGameEventSequence(UXD_ObjectFunctionLibrary::DuplicateObject(NextSequence.Get(), OwingEventFlow));
		}
		else
		{
			OwingEventFlow->SetAndActiveNextGameEventSequence(nullptr);
		}
	}
}

#undef LOCTEXT_NAMESPACE
