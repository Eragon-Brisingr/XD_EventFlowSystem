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

	FString GetVarRefName() const override;

	UXD_EventFlowSequenceBase* CreateInstanceByTemplate(UObject* Outer) const override;
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

	UFUNCTION(BlueprintCallable, Category = "游戏性|游戏事件")
	bool IsFinished() const;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = "游戏事件", ReplicatedUsing = OnRep_EventFlowElementList, SaveGame)
	TArray<UXD_EventFlowElementBase*> EventFlowElementList;
	UFUNCTION()
	void OnRep_EventFlowElementList();
	UPROPERTY()
	TArray<UXD_EventFlowElementBase*> PreEventFlowElementList;

	UPROPERTY(ReplicatedUsing = OnRep_SequenceTemplate, SaveGame)
	const UXD_EventFlowSequenceBase* SequenceTemplate;
	UFUNCTION()
	void OnRep_SequenceTemplate();

	virtual void TryBindRefAndDelegate(UXD_EventFlowBase* EventFlow);

	UPROPERTY(BlueprintReadOnly, Category = "游戏性|游戏事件")
	class UXD_EventFlowBase* OwingEventFlow;

	void ActiveEventFlowSequence();
	void DeactiveEventFlowSequence();
	void InitEventFlowSequence();
	void FinishEventFlowSequence();

	bool HasMustEventFlowElement();

#if WITH_EDITORONLY_DATA
	uint8 bIsSequenceActived : 1;
#endif
	void InvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag);
	//任务元素向任务序列申请完成该序列，是否完成交由该任务序列判断
	virtual void WhenInvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag);

	//当游戏事件元素从完成变为未完成 e.g.需收集的道具开始达到要求，之后被减少了
	virtual void WhenEventFlowElementUnfinished(){}

	bool IsEveryMustEventFlowElementFinished() const;

	virtual void DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch);

	UFUNCTION(BlueprintPure, Category = "游戏性|游戏事件")
	class APawn* GetEventFlowOwnerCharacter() const;
protected:
	virtual void WhenActiveEventFlowSequence() {}

	virtual void WhenDeactiveEventFlowSequence();
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSequenceInited, UXD_EventFlowSequenceBase*, Sequence);
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "序列初始化"))
	FOnSequenceInited OnSequenceInited;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSequenceActived, UXD_EventFlowSequenceBase*, Sequence);
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "序列激活"))
	FOnSequenceActived OnSequenceActived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSequenceDeactived, UXD_EventFlowSequenceBase*, Sequence);
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "序列反激活"))
	FOnSequenceDeactived OnSequenceDeactived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSequenceFinished, UXD_EventFlowSequenceBase*, Sequence);
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "序列结束"))
	FOnSequenceFinished OnSequenceFinished;
};

USTRUCT(BlueprintType)
struct XD_EVENTFLOWSYSTEM_API FEventFlowElementFinishWrapper
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "游戏事件", SaveGame)
	class UXD_EventFlowElementBase* EventFlowElement;

	UPROPERTY(NotReplicated)
	TMap<FName, UXD_EventFlowSequenceBase*> EventFlowFinishBranch;
};

//完成必须的游戏事件元素之后出现分支
UCLASS(meta = (DisplayName = "分支型序列"))
class XD_EVENTFLOWSYSTEM_API UEventFlowSequence_Branch : public UXD_EventFlowSequenceBase
{
	GENERATED_BODY()
public:
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const;

	void ReplicatedEventFlowElement(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags);

public:
	UPROPERTY(BlueprintReadOnly, Category = "游戏性|游戏事件")
	uint8 bIsFinishListActive : 1;

	void ReinitEventFlowSequence() override;

	void WhenActiveEventFlowSequence() override;

	void WhenDeactiveEventFlowSequence() override;

	void WhenInvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag) override;

	void WhenEventFlowElementUnfinished() override;

	void DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch) override;

	UEventFlowSequence_Branch* CreateInstanceByTemplate(UObject* Outer) const override;

	void TryBindRefAndDelegate(UXD_EventFlowBase* EventFlow) override;
public:
	void InvokeActiveFinishList();

	void DeactiveFinishBranchs();
public:
	UPROPERTY(BlueprintReadWrite, Category = "游戏性|游戏事件", ReplicatedUsing = OnRep_EventFlowElementFinishList, SaveGame)
	TArray<FEventFlowElementFinishWrapper> EventFlowElementFinishList;
	UFUNCTION()
	void OnRep_EventFlowElementFinishList();
};

//完成所有必须的游戏事件元素之后即进行下一个序列
UCLASS(meta = (DisplayName = "顺序型序列"))
class XD_EVENTFLOWSYSTEM_API UEventFlowSequence_List : public UXD_EventFlowSequenceBase
{
	GENERATED_BODY()
public:
	void WhenInvokeFinishEventFlowSequence(UXD_EventFlowElementBase* EventFlowElement, const FName& NextBranchTag) override;
public:
 	UPROPERTY()
 	UXD_EventFlowSequenceBase* NextSequenceTemplate;
};

