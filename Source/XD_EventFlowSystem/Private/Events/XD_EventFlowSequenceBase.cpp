// Fill out your copyright notice in the Description page of Project Settings.

#include "XD_EventFlowSequenceBase.h"
#include "XD_EventFlowBase.h"
#include "XD_EventFlowSystem_Log.h"
#include "XD_ObjectFunctionLibrary.h"
#include "XD_DebugFunctionLibrary.h"

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

	DOREPLIFETIME(UXD_EventFlowSequenceBase, EventFlowElementList);
}

void UXD_EventFlowSequenceBase::ReplicatedEventFlowElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		if (EventFlowElement)
		{
			WroteSomething |= Channel->ReplicateSubobject(EventFlowElement, *Bunch, *RepFlags);
		}
	}
}

UEventFlowGraphNodeBase* UXD_EventFlowSequenceBase::GetDuplicatedNode(UObject* Outer) const
{
	UXD_EventFlowSequenceBase* Sequence = (UXD_EventFlowSequenceBase*)Super::GetDuplicatedNode(Outer);
	Sequence->OnRep_EventFlowElementList();
	return Sequence;
}

void UXD_EventFlowSequenceBase::ActiveEventFlowSequence()
{
	EventFlowSystem_Display_Log("%s激活[%s]中的游戏事件序列[%s]", *UXD_DebugFunctionLibrary::GetDebugName(GetEventFlowOwnerCharacter()), *OwingEventFlow->GetEventFlowName().ToString(), *GetDescribe().ToString());
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		EventFlowElement->ActivateEventFlowElement();
	}
}

bool UXD_EventFlowSequenceBase::HasMustEventFlowElement()
{
	for (auto EventFlowElement : EventFlowElementList)
	{
		if (EventFlowElement->bIsMust)
		{
			return true;
		}
	}
	return false;
}

void UXD_EventFlowSequenceBase::DeactiveEventFlowSequence()
{
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		EventFlowElement->UnactiveEventFlowElement();
	}
}

void UXD_EventFlowSequenceBase::InvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag)
{
	unimplemented();
}

bool UXD_EventFlowSequenceBase::IsEveryMustEventFlowElementFinished() const
{
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		if (EventFlowElement->bIsMust && !EventFlowElement->IsFinished())
		{
			return false;
		}
	}
	return true;
}

void UXD_EventFlowSequenceBase::DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch)
{
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		EventFlowElement->DrawHintInWorld(ARPG_HUD, Index, false);
	}
}

void UXD_EventFlowSequenceBase::AddEventFlowElement(class UXD_EventFlowElementBase* EventFlowElement)
{
	EventFlowElementList.Add(EventFlowElement);
}

void UXD_EventFlowSequenceBase::ReinitEventFlowSequence()
{
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		if (EventFlowElement)
		{
			EventFlowElement->OwingEventFlowSequence = this;
		}
	}
}

FText UXD_EventFlowSequenceBase::GetDescribe() const
{
	return DescribeDelegate.IsBound() ? DescribeDelegate.Execute() : Describe;
}

void UXD_EventFlowSequenceBase::OnRep_EventFlowElementList()
{
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		if (EventFlowElement)
		{
			EventFlowElement->OwingEventFlowSequence = this;
		}
	}
}

class APawn* UXD_EventFlowSequenceBase::GetEventFlowOwnerCharacter() const
{
	return OwingEventFlow->GetEventFlowOwnerCharacter();
}

void UEventFlowSequence_Branch::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEventFlowSequence_Branch, EventFlowElementFinishList);

}

void UEventFlowSequence_Branch::ReplicatedEventFlowElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	Super::ReplicatedEventFlowElement(WroteSomething, Channel, Bunch, RepFlags);
	for (const FEventFlowElementFinishWarpper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
	{
		if (EventFlowElementFinishWarpper.EventFlowElement)
		{
			WroteSomething |= Channel->ReplicateSubobject(EventFlowElementFinishWarpper.EventFlowElement, *Bunch, *RepFlags);
		}
	}
}

void UEventFlowSequence_Branch::ReinitEventFlowSequence()
{
	Super::ReinitEventFlowSequence();
	for (FEventFlowElementFinishWarpper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
	{
		if (EventFlowElementFinishWarpper.EventFlowElement)
		{
			EventFlowElementFinishWarpper.EventFlowElement->OwingEventFlowSequence = this;
		}
	}
}

void UEventFlowSequence_Branch::ActiveEventFlowSequence()
{
	Super::ActiveEventFlowSequence();
	InvokeActiveFinishList();
}

void UEventFlowSequence_Branch::DeactiveEventFlowSequence()
{
	Super::DeactiveEventFlowSequence();
	DeactiveFinishBranchs();
}

void UEventFlowSequence_Branch::InvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag)
{
	//激活结束游戏事件的关键游戏事件
	if (EventFlowElementList.Contains(EventFlowElement))
	{
		InvokeActiveFinishList();
	}
	//结束游戏事件
	else
	{
		UXD_EventFlowSequenceBase* FinishEventFlowSequence = OwingEventFlow->GetUnderwayEventFlowSequence();
		for (FEventFlowElementFinishWarpper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
		{
			if (EventFlowElement == EventFlowElementFinishWarpper.EventFlowElement)
			{
				if (EventFlowElementFinishWarpper.EventFlowFinishBranch.Num() > 0)
				{
					if (TSoftObjectPtr<UXD_EventFlowSequenceBase>* NextSequenceTemplate = EventFlowElementFinishWarpper.EventFlowFinishBranch.Find(NextBranchTag))
					{
						if (NextSequenceTemplate->IsValid())
						{
							OwingEventFlow->SetAndActiveNextEventFlowSequence((UXD_EventFlowSequenceBase*)NextSequenceTemplate->Get()->GetDuplicatedNode(OwingEventFlow));
						}
						else
						{
							EventFlowSystem_Error_Log("事件流[%s]结束序列[%s]中标签为[%s]的分支模板失效", *UXD_DebugFunctionLibrary::GetDebugName(OwingEventFlow), *UXD_DebugFunctionLibrary::GetDebugName(this), *NextBranchTag.ToString());
						}
					}
					else
					{
						EventFlowSystem_Error_Log("事件流[%s]结束序列[%s]中未找到标签[%s]的分支", *UXD_DebugFunctionLibrary::GetDebugName(OwingEventFlow), *UXD_DebugFunctionLibrary::GetDebugName(this), *NextBranchTag.ToString());
					}
				}
				else
				{
					OwingEventFlow->SetAndActiveNextEventFlowSequence(nullptr);
				}
				break;
			}
		}
	}
}

void UEventFlowSequence_Branch::WhenEventFlowElementReactive()
{
	DeactiveFinishBranchs();
}

void UEventFlowSequence_Branch::DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch)
{
	Super::DrawHintInWorld(ARPG_HUD, Index, IsFinishBranch);
	if (bIsFinishListActive)
	{
		for (FEventFlowElementFinishWarpper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
		{
			EventFlowElementFinishWarpper.EventFlowElement->DrawHintInWorld(ARPG_HUD, Index, true);
		}
	}
}

UEventFlowGraphNodeBase* UEventFlowSequence_Branch::GetDuplicatedNode(UObject* Outer) const
{
	UEventFlowSequence_Branch* Branch = (UEventFlowSequence_Branch*)Super::GetDuplicatedNode(Outer);
	Branch->OnRep_EventFlowElementFinishList();
	return Branch;
}

void UEventFlowSequence_Branch::InvokeActiveFinishList()
{
	if (IsEveryMustEventFlowElementFinished())
	{
		if (EventFlowElementFinishList.Num() == 0)
		{
			OwingEventFlow->FinishEventFlowSucceed();
		}
		else
		{
			if (bIsFinishListActive == false)
			{
				for (FEventFlowElementFinishWarpper& EventFlowElementFinish : EventFlowElementFinishList)
				{
					EventFlowElementFinish.EventFlowElement->ActivateEventFlowElement();
				}
				bIsFinishListActive = true;
			}
		}
	}
}

void UEventFlowSequence_Branch::DeactiveFinishBranchs()
{
	bIsFinishListActive = false;
	for (FEventFlowElementFinishWarpper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
	{
		EventFlowElementFinishWarpper.EventFlowElement->UnactiveEventFlowElement();
	}
}

void UEventFlowSequence_Branch::OnRep_EventFlowElementFinishList()
{
	for (FEventFlowElementFinishWarpper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
	{
		if (EventFlowElementFinishWarpper.EventFlowElement)
		{
			EventFlowElementFinishWarpper.EventFlowElement->OwingEventFlowSequence = this;
		}
	}
}

void UEventFlowSequence_List::InvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag)
{
	if (IsEveryMustEventFlowElementFinished())
	{
		if (NextSequenceTemplate.IsValid())
		{
			OwingEventFlow->SetAndActiveNextEventFlowSequence(UXD_ObjectFunctionLibrary::DuplicateObject(NextSequenceTemplate.Get(), OwingEventFlow));
		}
		else
		{
			OwingEventFlow->SetAndActiveNextEventFlowSequence(nullptr);
		}
	}
}

#undef LOCTEXT_NAMESPACE
