// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EventFlowGraphNodeBase.h"
#include "XD_EventFlowSequenceBase.generated.h"

class UXD_EventFlowElementBase;

/**
 * 
 */
UCLASS(Abstract, BlueprintType)
class XD_EVENTFLOWSYSTEM_API UXD_EventFlowSequenceBase : public UEventFlowGraphNodeBase
{
	GENERATED_BODY()
public:
	virtual class UWorld* GetWorld() const override;

	virtual bool IsSupportedForNetworking()const override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const;

	virtual void ReplicatedEventFlowElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags);

	UEventFlowGraphNodeBase* GetDuplicatedNode(UObject* Outer) const override;
public:
	void AddEventFlowElement(UXD_EventFlowElementBase* EventFlowElement);

	virtual void ReinitEventFlowSequence();
public:
	UPROPERTY(EditAnywhere, Category = "游戏事件")
	FText Describe;
	DECLARE_DYNAMIC_DELEGATE_RetVal(FText, FGetEventFlowSequenceDescDelegate);
	UPROPERTY()
	FGetEventFlowSequenceDescDelegate DescribeDelegate;

	UFUNCTION(BlueprintCallable, Category = "游戏性|游戏事件")
	FText GetDescribe() const;

	UPROPERTY(BlueprintReadOnly, Category = "游戏性|游戏事件")
	uint8 bIsFinishListActive : 1;

	UPROPERTY(BlueprintReadOnly, Category = "游戏性|游戏事件", ReplicatedUsing = OnRep_EventFlowElementList, SaveGame)
	TArray<UXD_EventFlowElementBase*> EventFlowElementList;
	UFUNCTION()
	void OnRep_EventFlowElementList();

	UPROPERTY(BlueprintReadOnly, Category = "游戏性|游戏事件")
	class UXD_EventFlowBase* OwingEventFlow;

	virtual void ActiveEventFlowSequence();

	bool HasMustEventFlowElement();

	virtual void DeactiveEventFlowSequence();

	//任务元素向任务序列申请完成该序列，是否完成交由该任务序列判断
	virtual void InvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag);

	//当游戏事件元素从完成变为未完成 e.g.需收集的道具开始达到要求，之后被减少了
	virtual void WhenEventFlowElementReactive(){}

	bool IsEveryMustEventFlowElementFinished() const;

	virtual void DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch);
public:
	UFUNCTION(BlueprintPure, Category = "游戏性|游戏事件")
	class APawn* GetEventFlowOwnerCharacter() const;
};

USTRUCT(BlueprintType)
struct XD_EVENTFLOWSYSTEM_API FEventFlowElementFinishWarpper
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "游戏事件", SaveGame)
	class UXD_EventFlowElementBase* EventFlowElement;

	UPROPERTY(SaveGame)
	TMap<FName, TSoftObjectPtr<UXD_EventFlowSequenceBase>> EventFlowFinishBranch;
};

//完成必须的游戏事件元素之后出现分支
UCLASS(meta = (DisplayName = "分支型序列"))
class XD_EVENTFLOWSYSTEM_API UEventFlowSequence_Branch : public UXD_EventFlowSequenceBase
{
	GENERATED_BODY()
public:
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const;

	void ReplicatedEventFlowElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags);

	void ReinitEventFlowSequence() override;

	void ActiveEventFlowSequence() override;

	void DeactiveEventFlowSequence() override;

	void InvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag) override;

	void WhenEventFlowElementReactive() override;

	void DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch) override;

	UEventFlowGraphNodeBase* GetDuplicatedNode(UObject* Outer) const override;
public:
	void InvokeActiveFinishList();

	void DeactiveFinishBranchs();
public:
	UPROPERTY(BlueprintReadWrite, Category = "游戏性|游戏事件", ReplicatedUsing = OnRep_EventFlowElementFinishList, SaveGame)
	TArray<FEventFlowElementFinishWarpper> EventFlowElementFinishList;
	UFUNCTION()
	void OnRep_EventFlowElementFinishList();
};

//完成所有必须的游戏事件元素之后即进行下一个序列
UCLASS(meta = (DisplayName = "顺序型序列"))
class XD_EVENTFLOWSYSTEM_API UEventFlowSequence_List : public UXD_EventFlowSequenceBase
{
	GENERATED_BODY()
public:
	void InvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag) override;
public:
 	UPROPERTY(SaveGame)
 	TSoftObjectPtr<class UXD_EventFlowSequenceBase> NextSequenceTemplate;
};

