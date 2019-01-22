// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"
#include "EventFlowGraphBlueprint.generated.h"

class UEventFlowGraphBlueprint;
class UEventFlowGraphNodeBase;

/**
 * 
 */
USTRUCT()
struct XD_EVENTFLOWSYSTEM_API FEventFlowDelegateEditorBinding
{
	GENERATED_BODY()
public:
	/** The member widget the binding is on, must be a direct variable of the UUserWidget. */
	UPROPERTY()
	TSoftObjectPtr<UObject> Object;

	UPROPERTY()
	FName PropertyName;

	/** If it's an actual Function Graph in the blueprint that we're bound to, there's a GUID we can use to lookup that function, to deal with renames better.  This is that GUID. */
	UPROPERTY()
	FGuid MemberFunctionGuid;

	bool DoesBindingTargetExist(UEventFlowGraphBlueprint* Blueprint) const;

	FEventFlowDelegateRuntimeBinding ToRuntimeBinding(UEventFlowGraphBlueprint* Blueprint) const;

	FName GetFunctionName(UEventFlowGraphBlueprint* Blueprint) const;
};


UCLASS()
class XD_EVENTFLOWSYSTEM_API UEventFlowGraphBlueprint : public UBlueprint
{
	GENERATED_BODY()
public:
	UEventFlowGraphBlueprint();

	UClass* GetBlueprintClass() const override;
	void GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const override;
public:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UEdGraph* EdGraph;

	UPROPERTY()
	class UXD_EventFlowSequenceBase* StartSequence;

	UPROPERTY()
	TArray<FEventFlowDelegateEditorBinding> Bindings;

	TArray<UEventFlowGraphNodeBase*> GetAllNodes() const;
#endif
};
