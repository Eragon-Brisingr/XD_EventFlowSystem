// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XD_SaveGameInterface.h"
#include "XD_EventFlowManager.generated.h"

class UXD_EventFlowBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XD_EVENTFLOWSYSTEM_API UXD_EventFlowManager : public UActorComponent, public IXD_SaveGameInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXD_EventFlowManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override; 

	virtual void WhenPostLoad_Implementation() override;
private:
	void ReplicatedEventFlowList(const TArray<UXD_EventFlowBase*>& EventFlowList, bool& WroteSomething, class UActorChannel * Channel, class FOutBunch * Bunch, FReplicationFlags * RepFlags);
	//游戏事件
public:
	UPROPERTY()
	TArray<class UXD_EventFlowBase*> PreUnderwayEventFlowList;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "游戏事件", ReplicatedUsing = OnRep_UnderwayEventFlowList, SaveGame)
	TArray<class UXD_EventFlowBase*> UnderwayEventFlowList;
	UFUNCTION()
	void OnRep_UnderwayEventFlowList();

	UPROPERTY()
	TArray<class UXD_EventFlowBase*> PreFinishEventFlowList;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "游戏事件", ReplicatedUsing = OnRep_FinishEventFlowList, SaveGame)
	TArray<class UXD_EventFlowBase*> FinishEventFlowList;
	UFUNCTION()
	void OnRep_FinishEventFlowList();

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件", BlueprintAuthorityOnly)
	void ActiveEventFlow(TSubclassOf<UXD_EventFlowBase> EventFlowGraph);

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	bool IsEventFlowExistInUnderwayList(TSubclassOf<UXD_EventFlowBase> EventFlowGraph);

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	bool IsEventFlowExistInFinishList(TSubclassOf<UXD_EventFlowBase> EventFlowGraph);

	UFUNCTION(BlueprintCallable, Category = "角色|游戏事件")
	bool IsEventFlowExist(TSubclassOf<UXD_EventFlowBase> EventFlowGraph);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAcceptEventFlow, class UXD_EventFlowBase*, EventFlow);
	UPROPERTY(BlueprintAssignable, Category = "角色|游戏事件")
	FOnAcceptEventFlow OnEventFlowActived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemoveUnderwayEventFlow, class UXD_EventFlowBase*, EventFlow);
	UPROPERTY(BlueprintAssignable, Category = "角色|游戏事件")
	FOnRemoveUnderwayEventFlow OnUnderwayEventFlowRemoved;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFinishEventFlow, class UXD_EventFlowBase*, EventFlow);
	UPROPERTY(BlueprintAssignable, Category = "角色|游戏事件")
	FOnFinishEventFlow OnEventFlowFinished;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemoveFinishEventFlow, class UXD_EventFlowBase*, EventFlow);
	UPROPERTY(BlueprintAssignable, Category = "角色|游戏事件")
	FOnRemoveFinishEventFlow OnFinishEventFlowRemoved;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFinishedEventFlowSequence, class UXD_EventFlowBase*, EventFlow, class UXD_EventFlowSequenceBase*, FinishedEventFlowSequence, class UXD_EventFlowSequenceBase*, UnderwayEventFlowSequences);
	UPROPERTY(BlueprintAssignable, Category = "角色|游戏事件")
	FOnFinishedEventFlowSequence OnEventFlowSequenceFinished;
};
