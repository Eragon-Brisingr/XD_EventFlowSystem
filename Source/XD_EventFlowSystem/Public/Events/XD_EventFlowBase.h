// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EventFlowGraph.h"
#include "XD_EventFlowBase.generated.h"

class UEventFlowGraph;

/**
 * 
 */
UENUM(BlueprintType)
enum class EEventFlowState : uint8
{
	Underway UMETA(DisplayName = "进行中"),
	Finish_Succeed UMETA(DisplayName = "成功"),
	Finish_Failed UMETA(DisplayName = "失败")
};

USTRUCT(BlueprintType)
struct XD_EVENTFLOWSYSTEM_API FEventFlowElementFinishWarpper
{
	GENERATED_USTRUCT_BODY()

public:
	FEventFlowElementFinishWarpper() = default;

	FEventFlowElementFinishWarpper(class UXD_EventFlowElementBase* GameEventElement);

	FEventFlowElementFinishWarpper(class UXD_EventFlowElementBase* GameEventElement, const TSoftObjectPtr<class UXD_GameEventGraphNode>& GameEventFinishBranch);

public:
	UPROPERTY(BlueprintReadWrite, Category = "游戏事件", SaveGame)
	class UXD_EventFlowElementBase* GameEventElement;

// 	UPROPERTY(SaveGame)
// 	TSoftObjectPtr<class UXD_GameEventGraphNode> GameEventFinishBranch;
};


UCLASS(Blueprintable, BlueprintType, hidedropdown)
class XD_EVENTFLOWSYSTEM_API UXD_EventFlowBase : public UObject
{
	GENERATED_BODY()

public:
	UXD_EventFlowBase();

// 	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext), Category = "角色|游戏事件")
// 	static UXD_EventFlowBase* NewGameEvent(UObject* WorldContextObject, class UXD_GameEventGraph* GameEventGraph);

	void ReinitGameEvent(class UXD_EventFlowManager* GameEventOwner);

	void ReactiveGameEvent();
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
	struct FWorldContext* WorldContext;

	virtual class UWorld* GetWorld() const override;

	virtual bool IsSupportedForNetworking() const override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const;

	virtual void ReplicatedGameEventSequence(bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags);
public:
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "游戏事件", Replicated)
	EEventFlowState GameEventState;
	
	UPROPERTY(EditAnywhere, Category = "游戏事件")
	FText EventFlowName;

	//为真则显示于玩家面板上
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "游戏事件", Meta = (ExposeOnSpawn = true), Replicated)
	uint8 bIsShowGameEvent : 1;

public:
	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	bool IsFinished() const { return GameEventState != EEventFlowState::Underway; }

	const UEventFlowGraph* GetEventFlowGraphTemplate() const;

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	FText GetGameEventName() const;
public:
	UPROPERTY(BlueprintReadOnly, Category = "游戏事件", ReplicatedUsing = OnRep_CurrentGameEventSequenceList, SaveGame)
	TArray<UXD_EventFlowSequenceBase*> CurrentGameEventSequenceList;
	UFUNCTION()
	void OnRep_CurrentGameEventSequenceList();

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	FORCEINLINE UXD_EventFlowSequenceBase* GetUnderwayGameEventSequence() const
	{
		return CurrentGameEventSequenceList[CurrentGameEventSequenceList.Num() - 1];
	}

	//NextGameEventSequence == nullptr 则完成该任务
	void SetAndActiveNextGameEventSequence(class UXD_EventFlowSequenceBase* NextGameEventSequence);

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	class APawn* GetGameEventOwnerCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	class AController* GeGameEventOwnerController() const;

	UPROPERTY(BlueprintReadOnly, Category = "角色|游戏事件")
	class UXD_EventFlowManager* GameEventOwner;
public:
	void ActiveGameEvent(class UXD_EventFlowManager* GameEventExecuter);

	void FinishGameEvent();
	
	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	void ForceFinishGameEvent(EEventFlowState State = EEventFlowState::Finish_Failed);

	UFUNCTION(BlueprintNativeEvent, Category = "游戏事件")
	void WhenFinishGameEvent();
	virtual void WhenFinishGameEvent_Implementation(){}

	UFUNCTION(BlueprintNativeEvent, Category = "游戏事件")
	void WhenFinishedGameEventSequence(class UXD_EventFlowSequenceBase* FinishedGameEventSequence, class UXD_EventFlowSequenceBase* UnderwayGameEventSequences);
	virtual void WhenFinishedGameEventSequence_Implementation(class UXD_EventFlowSequenceBase* FinishedGameEventSequence, class UXD_EventFlowSequenceBase* UnderwayGameEventSequences);

	UFUNCTION(BlueprintNativeEvent, Category = "游戏事件")
	void InitGameEvent();
	void InitGameEvent_Implementation(){}
};
