// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "XD_EventFlowBase.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EEventFlowState : uint8
{
	Underway UMETA(DisplayName = "进行中"),
	Blocked UMETA(DisplayName = "中断"),
	Finish_Succeed UMETA(DisplayName = "成功"),
	Finish_Failed UMETA(DisplayName = "失败")
};

UCLASS(Abstract, BlueprintType, hidedropdown)
class XD_EVENTFLOWSYSTEM_API UXD_EventFlowBase : public UObject
{
	GENERATED_BODY()

public:
	UXD_EventFlowBase();

	void ReinitEventFlow(class UXD_EventFlowManager* EventFlowOwner);

	void ReactiveEventFlow();
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
	virtual class UWorld* GetWorld() const override;

	virtual bool IsSupportedForNetworking() const override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const;

	virtual void ReplicatedEventFlowSequence(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags);
public:
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "游戏事件", Replicated)
	EEventFlowState EventFlowState;
	
	UPROPERTY(EditAnywhere, Category = "游戏事件")
	FText EventFlowName;

	//为真则显示于玩家面板上
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "游戏事件", Meta = (ExposeOnSpawn = true), Replicated)
	uint8 bIsShowEventFlow : 1;
public:
	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	bool IsFinished() const { return EventFlowState == EEventFlowState::Finish_Succeed || EventFlowState == EEventFlowState::Finish_Failed; }

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	FText GetEventFlowName() const;
public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "游戏事件", ReplicatedUsing = OnRep_CurrentEventFlowSequenceList, SaveGame)
	TArray<UXD_EventFlowSequenceBase*> CurrentEventFlowSequenceList;
	UFUNCTION()
	void OnRep_CurrentEventFlowSequenceList();
	UPROPERTY()
	TArray<UXD_EventFlowSequenceBase*> PreEventFlowSequenceList;

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	FORCEINLINE UXD_EventFlowSequenceBase* GetUnderwayEventFlowSequence() const
	{
		return CurrentEventFlowSequenceList[CurrentEventFlowSequenceList.Num() - 1];
	}

	//NextEventFlowSequence == nullptr 则完成该任务
	void SetAndActiveNextEventFlowSequence(class UXD_EventFlowSequenceBase* NextEventFlowSequence);

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	class APawn* GetEventFlowOwnerCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	class AController* GetEventFlowOwnerController() const;

	UPROPERTY(BlueprintReadOnly, Category = "角色|游戏事件")
	class UXD_EventFlowManager* EventFlowOwner;
public:
	void ActiveEventFlow(class UXD_EventFlowManager* EventFlowExecuter);

	void FinishEventFlowSucceed();
	
	void FinishEventFlowFailed();

	UFUNCTION(BlueprintNativeEvent, Category = "游戏事件")
	void WhenFinishEventFlow();
	virtual void WhenFinishEventFlow_Implementation(){}

	UFUNCTION(BlueprintNativeEvent, Category = "游戏事件")
	void WhenFinishedEventFlowSequence(class UXD_EventFlowSequenceBase* FinishedEventFlowSequence, class UXD_EventFlowSequenceBase* UnderwayEventFlowSequences);
	virtual void WhenFinishedEventFlowSequence_Implementation(class UXD_EventFlowSequenceBase* FinishedEventFlowSequence, class UXD_EventFlowSequenceBase* UnderwayEventFlowSequences);

	UFUNCTION(BlueprintNativeEvent, Category = "游戏事件")
	void InitEventFlow();
	void InitEventFlow_Implementation() {}

private:
	void AddNextSequence(class UXD_EventFlowSequenceBase* NextEventFlowSequence);
};
