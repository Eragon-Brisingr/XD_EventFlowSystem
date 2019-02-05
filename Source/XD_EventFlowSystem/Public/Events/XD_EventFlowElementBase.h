// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EventFlowGraphNodeBase.h"
#include <UserWidget.h>
#include "XD_EventFlowElementBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable, collapseCategories)
class XD_EVENTFLOWSYSTEM_API UXD_EventFlowElementBase : public UEventFlowGraphNodeBase
{
	GENERATED_BODY()

public:
	UXD_EventFlowElementBase();

	class UWorld* GetWorld() const override;

	bool IsSupportedForNetworking()const override;

	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FString GetVarRefName() const override;

	UXD_EventFlowElementBase* GetDuplicatedNode(UObject* Outer) const override;

	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_ElementTemplate)
	const UXD_EventFlowElementBase* ElementTemplate;
	UFUNCTION()
	void OnRep_ElementTemplate();

	void TryBindRefAndDelegate(UXD_EventFlowBase* EventFlow);
private:
	UPROPERTY(SaveGame, Replicated)
	uint8 bIsFinished : 1;
public:
	//为真则为必要游戏事件
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "高级", Meta = (DisplayName = "必要游戏事件", ExposeOnSpawn = true), Replicated)
	uint8 bIsMust : 1;

	//为真则显示于玩家面板上
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "高级", Meta = (DisplayName = "显示于玩家面板", ExposeOnSpawn = true), Replicated)
	uint8 bIsShowEventFlowElement : 1;
public:
	//获取游戏事件元素描述
	UFUNCTION(BlueprintPure, Category = "游戏事件")
	FText GetDescribe();

	UFUNCTION(BlueprintNativeEvent, Category="游戏事件", meta = (DisplayName = "GetDescribe"))
	FText ReceiveGetDescribe() const;
	virtual FText ReceiveGetDescribe_Implementation() const;

	//显示游戏事件提示在指南针上
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "游戏事件")
	void DrawHintOnHUD(const FPaintContext& PaintContext, const FGeometry& Geometry, APlayerController* Controller, int32 Index, bool IsFinishBranch);
	virtual void DrawHintOnHUD_Implementation(const FPaintContext& PaintContext, const FGeometry& Geometry, APlayerController* Controller, int32 Index, bool IsFinishBranch) {};

	//显示游戏事件提示在世界中
	UFUNCTION(BlueprintNativeEvent, Category = "游戏事件")
	void DrawHintInWorld(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch);
	virtual void DrawHintInWorld_Implementation(class AHUD* ARPG_HUD, int32 Index, bool IsFinishBranch){}

	//游戏事件是否完成
	UFUNCTION(BlueprintCallable, Category = "游戏事件")
	FORCEINLINE bool IsFinished() const { return bIsFinished; }

	/**
	* 完成该任务序列
	* @param	NextBranchTag		游戏事件元素中可能也存在分支，比如说和某人对话中出现的分支，用NextBranchTag区分
	*/
	UFUNCTION(BlueprintCallable, Category = "游戏事件", meta = (AdvancedDisplay = "0"))
	void FinishEventFlowElement(const FName& NextBranchTag = NAME_None);

	//该游戏元素又未完成了调用这个 e.g.目标数量道具减少
	UFUNCTION(BlueprintCallable, Category = "游戏事件")
	void SetUnfinished();
public:
#if WITH_EDITORONLY_DATA
	uint8 bIsActive : 1;
#endif
	void ActivateEventFlowElement();
	//用于激活该游戏事件元素的检查事件
	UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, Category = "游戏事件")
	void WhenActivateEventFlowElement(class APawn* EventFlowOwnerCharacter, class AController* EventFlowOwner);
	virtual void WhenActivateEventFlowElement_Implementation(class APawn* EventFlowOwnerCharacter, class AController* EventFlowOwner){}

	void DeactiveEventFlowElement();
	//用于反激活该游戏事件元素的检查事件
	UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, Category = "游戏事件")
	void WhenDeactiveEventFlowElement(class APawn* EventFlowOwnerCharacter, class AController* EventFlowOwner);
	virtual void WhenDeactiveEventFlowElement_Implementation(class APawn* EventFlowOwnerCharacter, class AController* EventFlowOwner){}
public:
	UPROPERTY(BlueprintReadOnly, Category = "游戏事件")
	class UXD_EventFlowSequenceBase* OwingEventFlowSequence;

	UFUNCTION(BlueprintPure, Category = "游戏事件")
	class AController* GetOwingController() const;
	UFUNCTION(BlueprintPure, Category = "游戏事件")
	class APawn* GetOwningCharacter() const;
	UFUNCTION(BlueprintPure, Category = "游戏事件")
	class UXD_EventFlowBase* GetEventFlow() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnElementActived, UXD_EventFlowElementBase*, Element);
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "元素激活"))
	FOnElementActived OnElementActived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnElementDeactived, UXD_EventFlowElementBase*, Element);
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "元素反激活"))
	FOnElementDeactived OnElementDeactived;
};

UCLASS(meta = (DisplayName = "调试"))
class XD_EVENTFLOWSYSTEM_API UElement_Debug : public UXD_EventFlowElementBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "调试", meta = (DisplayName = "完成时间"))
	float FinishTime = 1.f;

	FTimerHandle TimerHandle;

	void WhenActivateEventFlowElement_Implementation(class APawn* EventFlowOwnerCharacter, class AController* EventFlowOwner) override;

	void WhenDeactiveEventFlowElement_Implementation(class APawn* EventFlowOwnerCharacter, class AController* EventFlowOwner) override;

	void FinishEventFlowElement();
};