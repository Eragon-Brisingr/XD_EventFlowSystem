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
	DOREPLIFETIME(UXD_EventFlowSequenceBase, SequenceTemplate);
}

void UXD_EventFlowSequenceBase::ReplicatedEventFlowElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		if (EventFlowElement)
		{
			WroteSomething |= Channel->ReplicateSubobject(EventFlowElement, *Bunch, *RepFlags);
			EventFlowElement->ReplicatedElementSubobject(WroteSomething, Channel, Bunch, RepFlags);
		}
	}
}

FString UXD_EventFlowSequenceBase::GetVarRefName() const
{
	if (SequenceTemplate)
	{
		return FString::Printf(TEXT("Ref_%s"), *SequenceTemplate->GetName());
	}
	else
	{
		return FString::Printf(TEXT("Ref_%s"), *GetName());
	}
}

UXD_EventFlowSequenceBase* UXD_EventFlowSequenceBase::CreateInstanceByTemplate(UObject* Outer) const
{
	UXD_EventFlowSequenceBase* Sequence = CastChecked<UXD_EventFlowSequenceBase>(Super::CreateInstanceByTemplate(Outer));
	Sequence->OwingEventFlow = Cast<UXD_EventFlowBase>(Outer);
	Sequence->SequenceTemplate = this;
	for (int32 Idx = 0; Idx < EventFlowElementList.Num(); ++Idx)
	{
		Sequence->EventFlowElementList[Idx] = EventFlowElementList[Idx]->CreateInstanceByTemplate(Sequence);
	}
	Sequence->OnRep_EventFlowElementList();
	return Sequence;
}

void UXD_EventFlowSequenceBase::OnRep_SequenceTemplate()
{
	if (OwingEventFlow)
	{
		TryBindRefAndDelegate(OwingEventFlow);
	}
}

void UXD_EventFlowSequenceBase::TryBindRefAndDelegate(UXD_EventFlowBase* EventFlow)
{
	if (SequenceTemplate && SequenceTemplate->bIsVariable)
	{
		BindRefAndDelegate(EventFlow);
	}

	for (UXD_EventFlowElementBase* Element : EventFlowElementList)
	{
		if (Element)
		{
			Element->TryBindRefAndDelegate(EventFlow);
		}
	}
}

void UXD_EventFlowSequenceBase::ActiveEventFlowSequence()
{
#if WITH_EDITOR
	check(bIsSequenceActived == false);
	bIsSequenceActived = true;
#endif
	EventFlowSystem_Display_Log("%s激活[%s]中的游戏事件序列%s", *UXD_DebugFunctionLibrary::GetDebugName(GetEventFlowOwnerCharacter()), *OwingEventFlow->GetEventFlowName().ToString(), *UXD_DebugFunctionLibrary::GetDebugName(this));
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		EventFlowElement->ActivateEventFlowElement();
	}
	WhenActiveEventFlowSequence();
	OnSequenceActived.Broadcast(this);
}

void UXD_EventFlowSequenceBase::DeactiveEventFlowSequence()
{
#if WITH_EDITOR
	check(bIsSequenceActived);
	bIsSequenceActived = false;
#endif
	EventFlowSystem_Display_Log("%s停止激活[%s]中的游戏事件序列%s", *UXD_DebugFunctionLibrary::GetDebugName(GetEventFlowOwnerCharacter()), *OwingEventFlow->GetEventFlowName().ToString(), *UXD_DebugFunctionLibrary::GetDebugName(this));
	for (UXD_EventFlowElementBase* EventFlowElement : EventFlowElementList)
	{
		EventFlowElement->DeactiveEventFlowElement();
	}
	WhenDeactiveEventFlowSequence();
	OnSequenceDeactived.Broadcast(this);
}

void UXD_EventFlowSequenceBase::InitEventFlowSequence()
{
	OnSequenceInited.Broadcast(this);
}

void UXD_EventFlowSequenceBase::FinishEventFlowSequence()
{
	EventFlowSystem_Display_Log("%s完成[%s]中的游戏事件序列%s", *UXD_DebugFunctionLibrary::GetDebugName(GetEventFlowOwnerCharacter()), *OwingEventFlow->GetEventFlowName().ToString(), *UXD_DebugFunctionLibrary::GetDebugName(this));
	OnSequenceFinished.Broadcast(this);
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

void UXD_EventFlowSequenceBase::WhenDeactiveEventFlowSequence()
{

}

void UXD_EventFlowSequenceBase::InvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag)
{
#if WITH_EDITOR
	if (ensure(bIsSequenceActived) == false)
	{
		EventFlowSystem_Error_Log("任务元素%s触发了已经结束的序列%s，任务元素反激活的代码存在问题", *UXD_DebugFunctionLibrary::GetDebugName(EventFlowElement->GetClass()), *UXD_DebugFunctionLibrary::GetDebugName(this));
		return;
	}
#endif
	WhenInvokeFinishEventFlowSequence(EventFlowElement, NextBranchTag);
}

void UXD_EventFlowSequenceBase::WhenInvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag)
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
	return DescribeDelegate.IsBound() ? DescribeDelegate.Execute() : SequenceTemplate ? SequenceTemplate->Describe : Describe;
}

bool UXD_EventFlowSequenceBase::IsFinished() const
{
	return OwingEventFlow ? OwingEventFlow->GetUnderwayEventFlowSequence() != this : false;
}

void UXD_EventFlowSequenceBase::OnRep_EventFlowElementList()
{
	for (UXD_EventFlowElementBase* AddedEventFlowElement : TSet<UXD_EventFlowElementBase*>(EventFlowElementList).Difference(TSet<UXD_EventFlowElementBase*>(PreEventFlowElementList)))
	{
		if (AddedEventFlowElement)
		{
			AddedEventFlowElement->OwingEventFlowSequence = this;

			if (OwingEventFlow)
			{
				AddedEventFlowElement->TryBindRefAndDelegate(OwingEventFlow);
			}
		}
	}

	PreEventFlowElementList = EventFlowElementList;
}

class APawn* UXD_EventFlowSequenceBase::GetEventFlowOwnerCharacter() const
{
	return OwingEventFlow ? OwingEventFlow->GetEventFlowOwnerCharacter() : nullptr;
}

void UEventFlowSequence_Branch::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEventFlowSequence_Branch, EventFlowElementFinishList);
}

void UEventFlowSequence_Branch::ReplicatedEventFlowElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags)
{
	Super::ReplicatedEventFlowElement(WroteSomething, Channel, Bunch, RepFlags);
	for (const FEventFlowElementFinishWrapper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
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
	for (FEventFlowElementFinishWrapper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
	{
		if (EventFlowElementFinishWarpper.EventFlowElement)
		{
			EventFlowElementFinishWarpper.EventFlowElement->OwingEventFlowSequence = this;
		}
	}
}

void UEventFlowSequence_Branch::WhenActiveEventFlowSequence()
{
	Super::WhenActiveEventFlowSequence();
	InvokeActiveFinishList();
}

void UEventFlowSequence_Branch::WhenDeactiveEventFlowSequence()
{
	Super::WhenDeactiveEventFlowSequence();
	DeactiveFinishBranchs();
}

void UEventFlowSequence_Branch::WhenInvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag)
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

		int32 Idx = EventFlowElementFinishList.IndexOfByPredicate([&](const FEventFlowElementFinishWrapper& EventFlowElementFinishWarpper) {return EventFlowElement == EventFlowElementFinishWarpper.EventFlowElement; });

		if (Idx != INDEX_NONE)
		{
			const FEventFlowElementFinishWrapper& EventFlowElementFinishWarpper = CastChecked<UEventFlowSequence_Branch>(SequenceTemplate)->EventFlowElementFinishList[Idx];
			if (EventFlowElementFinishWarpper.EventFlowFinishBranch.Num() > 0)
			{
				if (UXD_EventFlowSequenceBase*const* NextSequenceTemplate = EventFlowElementFinishWarpper.EventFlowFinishBranch.Find(NextBranchTag))
				{
					if (NextSequenceTemplate)
					{
						OwingEventFlow->SetAndActiveNextEventFlowSequence((*NextSequenceTemplate)->CreateInstanceByTemplate(OwingEventFlow));
					}
					else
					{
						EventFlowSystem_Error_Log("事件流[%s]结束序列%s中标签为[%s]的分支模板失效", *UXD_DebugFunctionLibrary::GetDebugName(OwingEventFlow), *UXD_DebugFunctionLibrary::GetDebugName(this), *NextBranchTag.ToString());
					}
				}
				else
				{
					EventFlowSystem_Error_Log("事件流[%s]结束序列%s中未找到标签[%s]的分支", *UXD_DebugFunctionLibrary::GetDebugName(OwingEventFlow), *UXD_DebugFunctionLibrary::GetDebugName(this), *NextBranchTag.ToString());
				}
			}
			else
			{
				OwingEventFlow->SetAndActiveNextEventFlowSequence(nullptr);
			}
		}
	}
}

void UEventFlowSequence_Branch::WhenEventFlowElementUnfinished()
{
	DeactiveFinishBranchs();
}

void UEventFlowSequence_Branch::DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch)
{
	Super::DrawHintInWorld(ARPG_HUD, Index, IsFinishBranch);
	if (bIsFinishListActive)
	{
		for (FEventFlowElementFinishWrapper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
		{
			EventFlowElementFinishWarpper.EventFlowElement->DrawHintInWorld(ARPG_HUD, Index, true);
		}
	}
}

UEventFlowSequence_Branch* UEventFlowSequence_Branch::CreateInstanceByTemplate(UObject* Outer) const
{
	UEventFlowSequence_Branch* Branch = CastChecked<UEventFlowSequence_Branch>(Super::CreateInstanceByTemplate(Outer));
	for (int32 Idx = 0; Idx < EventFlowElementFinishList.Num(); ++Idx)
	{
		Branch->EventFlowElementFinishList[Idx].EventFlowElement = EventFlowElementFinishList[Idx].EventFlowElement->CreateInstanceByTemplate(Branch);
	}
	Branch->OnRep_EventFlowElementFinishList();
	return Branch;
}

void UEventFlowSequence_Branch::TryBindRefAndDelegate(UXD_EventFlowBase* EventFlow)
{
	Super::TryBindRefAndDelegate(EventFlow);

	for (FEventFlowElementFinishWrapper& Wrapper : EventFlowElementFinishList)
	{
		if (Wrapper.EventFlowElement)
		{
			Wrapper.EventFlowElement->TryBindRefAndDelegate(EventFlow);
		}
	}
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
				for (FEventFlowElementFinishWrapper& EventFlowElementFinish : EventFlowElementFinishList)
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
	for (FEventFlowElementFinishWrapper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
	{
		EventFlowElementFinishWarpper.EventFlowElement->DeactiveEventFlowElement();
	}
}

void UEventFlowSequence_Branch::OnRep_EventFlowElementFinishList()
{
	for (FEventFlowElementFinishWrapper& EventFlowElementFinishWarpper : EventFlowElementFinishList)
	{
		if (EventFlowElementFinishWarpper.EventFlowElement)
		{
			EventFlowElementFinishWarpper.EventFlowElement->OwingEventFlowSequence = this;
		}
	}
}

void UEventFlowSequence_List::WhenInvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag)
{
	if (IsEveryMustEventFlowElementFinished())
	{
		if (UXD_EventFlowSequenceBase* NextSequence = Cast<UEventFlowSequence_List>(SequenceTemplate)->NextSequenceTemplate)
		{
			OwingEventFlow->SetAndActiveNextEventFlowSequence(NextSequence->CreateInstanceByTemplate(OwingEventFlow));
		}
		else
		{
			OwingEventFlow->SetAndActiveNextEventFlowSequence(nullptr);
		}
	}
}

#undef LOCTEXT_NAMESPACE
