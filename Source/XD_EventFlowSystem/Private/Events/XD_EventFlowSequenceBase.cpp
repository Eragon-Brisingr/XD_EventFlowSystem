// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_EventFlowSequenceBase.h"
#include "XD_EventFlowBase.h"
#include "XD_EventFlowSystem_Log.h"

#define LOCTEXT_NAMESPACE "游戏事件"

class UWorld* UXD_EventFlowSequenceBase::GetWorld() const
{
	return OwingGameEvent ? OwingGameEvent->GetWorld() : nullptr;
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
	EventFlowSystem_Display_LOG("%s激活[%s]中的游戏事件序列[%s]", *UXD_DebugFunctionLibrary::GetDebugName(GetGameEventOwnerCharacter()), *OwingGameEvent->GetGameEventName().ToString(), *GetDescribe().ToString());
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
			GameEventElement->OwingGameEventSequence = this;
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
			GameEventElement->OwingGameEventSequence = this;
		}
	}
}

class APawn* UXD_EventFlowSequenceBase::GetGameEventOwnerCharacter() const
{
	return OwingGameEvent->GetGameEventOwnerCharacter();
}

void UGameEventSequence_Branch::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGameEventSequence_Branch, GameEventElementFinishList);

}

void UGameEventSequence_Branch::ReplicatedGameEventElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	Super::ReplicatedGameEventElement(WroteSomething, Channel, Bunch, RepFlags);
	for (const FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
	{
		if (GameEventElementFinishWarpper.GameEventElement)
		{
			WroteSomething |= Channel->ReplicateSubobject(GameEventElementFinishWarpper.GameEventElement, *Bunch, *RepFlags);
		}
	}
}

void UGameEventSequence_Branch::ReinitGameEventSequence()
{
	Super::ReinitGameEventSequence();
	for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
	{
		if (GameEventElementFinishWarpper.GameEventElement)
		{
			GameEventElementFinishWarpper.GameEventElement->OwingGameEventSequence = this;
		}
	}
}

void UGameEventSequence_Branch::ActiveGameEventSequence()
{
	Super::ActiveGameEventSequence();
	InvokeActiveFinishList();
}

void UGameEventSequence_Branch::DeactiveGameEventSequence()
{
	Super::DeactiveGameEventSequence();
	DeactiveFinishBranchs();
}

void UGameEventSequence_Branch::InvokeFinishGameEventSequence(class UXD_EventFlowElementBase* GameEventElement, int32 Index)
{
	//激活结束游戏事件的关键游戏事件
	if (GameEventElementList.Contains(GameEventElement))
	{
		InvokeActiveFinishList();
	}
	//结束游戏事件
	else
	{
		UXD_EventFlowSequenceBase* FinishGameEventSequence = OwingGameEvent->GetUnderwayGameEventSequence();
		for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
		{
			if (GameEventElement == GameEventElementFinishWarpper.GameEventElement)
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

void UGameEventSequence_Branch::WhenGameEventElementReactive()
{
	DeactiveFinishBranchs();
}

void UGameEventSequence_Branch::DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch)
{
	Super::DrawHintInWorld(ARPG_HUD, Index, IsFinishBranch);
	if (bIsFinishListActive)
	{
		for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
		{
			GameEventElementFinishWarpper.GameEventElement->DrawHintInWorld(ARPG_HUD, Index, true);
		}
	}
}

void UGameEventSequence_Branch::InvokeActiveFinishList()
{
	if (IsEveryMustGameEventElementFinished())
	{
		if (GameEventElementFinishList.Num() == 0)
		{
			OwingGameEvent->FinishGameEvent();
		}
		else
		{
			if (bIsFinishListActive == false)
			{
				for (FEventFlowElementFinishWarpper& GameEventElementFinish : GameEventElementFinishList)
				{
					GameEventElementFinish.GameEventElement->ActivateGameEventElement();
				}
				bIsFinishListActive = true;
			}
		}
	}
}

void UGameEventSequence_Branch::DeactiveFinishBranchs()
{
	bIsFinishListActive = false;
	for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
	{
		GameEventElementFinishWarpper.GameEventElement->UnactiveGameEventElement();
	}
}

void UGameEventSequence_Branch::OnRep_GameEventElementFinishList()
{
	for (FEventFlowElementFinishWarpper& GameEventElementFinishWarpper : GameEventElementFinishList)
	{
		if (GameEventElementFinishWarpper.GameEventElement)
		{
			GameEventElementFinishWarpper.GameEventElement->OwingGameEventSequence = this;
		}
	}
}

void UGameEventSequence_List::InvokeFinishGameEventSequence(class UXD_EventFlowElementBase* GameEventElement, int32 Index)
{
	if (IsEveryMustGameEventElementFinished())
	{
// 		if (NextGameEvent.IsValid())
// 		{
// 			OwingGameEvent->SetAndActiveNextGameEventSequence(NextGameEvent->GetGameEventSequence(OwingGameEvent, NextEdge));
// 		}
// 		else
// 		{
// 			OwingGameEvent->SetAndActiveNextGameEventSequence(nullptr);
// 		}
	}
}

#undef LOCTEXT_NAMESPACE
