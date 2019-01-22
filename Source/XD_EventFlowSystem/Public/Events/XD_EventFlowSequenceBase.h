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

	virtual void ReplicatedGameEventElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags);
public:
	void AddGameEventElement(UXD_EventFlowElementBase* GameEventElement);

	virtual void ReinitGameEventSequence();
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

	UPROPERTY(BlueprintReadOnly, Category = "游戏性|游戏事件", ReplicatedUsing = OnRep_GameEventElementList, SaveGame)
	TArray<UXD_EventFlowElementBase*> GameEventElementList;
	UFUNCTION()
	void OnRep_GameEventElementList();

	UPROPERTY(BlueprintReadOnly, Category = "游戏性|游戏事件")
	class UXD_EventFlowBase* OwingEventFlow;

	virtual void ActiveGameEventSequence();

	bool HasMustGameEventElement();

	virtual void DeactiveGameEventSequence();

	//任务元素向任务序列申请完成该序列，是否完成交由该任务序列判断
	virtual void InvokeFinishGameEventSequence(UXD_EventFlowElementBase* GameEventElement, int32 Index);

	//当游戏事件元素从完成变为未完成 e.g.需收集的道具开始达到要求，之后被减少了
	virtual void WhenGameEventElementReactive(){}

	bool IsEveryMustGameEventElementFinished() const;

	virtual void DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch);
public:
	UFUNCTION(BlueprintPure, Category = "游戏性|游戏事件")
	class APawn* GetGameEventOwnerCharacter() const;
	 
};

USTRUCT(BlueprintType)
struct XD_EVENTFLOWSYSTEM_API FEventFlowElementFinishWarpper
{
	GENERATED_USTRUCT_BODY()

public:
	FEventFlowElementFinishWarpper(class UXD_EventFlowElementBase* EventFlowElement = nullptr, const TSoftObjectPtr<class UXD_GameEventGraphNode>& GameEventFinishBranch = nullptr)
		:EventFlowElement(nullptr)
	{}

public:
	UPROPERTY(BlueprintReadWrite, Category = "游戏事件", SaveGame)
	class UXD_EventFlowElementBase* EventFlowElement;

	// 	UPROPERTY(SaveGame)
	// 	TSoftObjectPtr<class UXD_GameEventGraphNode> GameEventFinishBranch;
};

//完成必须的游戏事件元素之后出现分支
UCLASS(meta = (DisplayName = "分支型序列"))
class XD_EVENTFLOWSYSTEM_API UEventFlowSequence_Branch : public UXD_EventFlowSequenceBase
{
	GENERATED_BODY()
public:
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const;

	void ReplicatedGameEventElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags);

	void ReinitGameEventSequence() override;

	void ActiveGameEventSequence() override;

	void DeactiveGameEventSequence() override;

	void InvokeFinishGameEventSequence(UXD_EventFlowElementBase* GameEventElement, int32 Index) override;

	void WhenGameEventElementReactive() override;

	void DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch) override;

	void InvokeActiveFinishList();

	void DeactiveFinishBranchs();
public:
	UPROPERTY(BlueprintReadWrite, Category = "游戏性|游戏事件", ReplicatedUsing = OnRep_GameEventElementFinishList, SaveGame)
	TArray<FEventFlowElementFinishWarpper> GameEventElementFinishList;
	UFUNCTION()
	void OnRep_GameEventElementFinishList();

};

//完成所有必须的游戏事件元素之后即进行下一个序列
UCLASS(meta = (DisplayName = "顺序型序列"))
class XD_EVENTFLOWSYSTEM_API UEventFlowSequence_List : public UXD_EventFlowSequenceBase
{
	GENERATED_BODY()
public:
 	UPROPERTY(SaveGame)
 	TSoftObjectPtr<class UXD_EventFlowSequenceBase> NextSequence;

	void InvokeFinishGameEventSequence(UXD_EventFlowElementBase* GameEventElement, int32 Index) override;
};

