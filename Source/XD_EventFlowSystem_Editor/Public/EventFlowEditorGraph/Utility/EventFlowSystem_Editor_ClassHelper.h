// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/WeakObjectPtr.h"
#include "EventFlowSystem_Editor_ClassHelper.generated.h"

/**
 * 
 */

USTRUCT()
struct FEventFlowSystem_Editor_ClassData
{
	GENERATED_USTRUCT_BODY()

	FEventFlowSystem_Editor_ClassData() {}
	FEventFlowSystem_Editor_ClassData(UClass* InClass, const FString& InDeprecatedMessage);
	FEventFlowSystem_Editor_ClassData(const FString& InAssetName, const FString& InGeneratedClassPackage, const FString& InClassName, UClass* InClass);

	FString ToString() const;
	FString GetClassName() const;
	FText GetCategory() const;
	FString GetDisplayName() const;
	UClass* GetClass(bool bSilent = false);
	bool IsAbstract() const;

	FORCEINLINE bool IsBlueprint() const { return AssetName.Len() > 0; }
	FORCEINLINE bool IsDeprecated() const { return DeprecatedMessage.Len() > 0; }
	FORCEINLINE FString GetDeprecatedMessage() const { return DeprecatedMessage; }
	FORCEINLINE FString GetPackageName() const { return GeneratedClassPackage; }

	/** set when child class masked this one out (e.g. always use game specific class instead of engine one) */
	uint32 bIsHidden : 1;

	/** set when class wants to hide parent class from selection (just one class up hierarchy) */
	uint32 bHideParent : 1;

private:

	/** pointer to uclass */
	TWeakObjectPtr<UClass> Class;

	/** path to class if it's not loaded yet */
	UPROPERTY()
	FString AssetName;

	UPROPERTY()
	FString GeneratedClassPackage;

	/** resolved name of class from asset data */
	UPROPERTY()
	FString ClassName;

	/** User-defined category for this class */
	UPROPERTY()
	FText Category;

	/** message for deprecated class */
	FString DeprecatedMessage;
};

struct FEventFlowEditor_ClassNode
{
	FEventFlowSystem_Editor_ClassData Data;
	FString ParentClassName;

	TSharedPtr<FEventFlowEditor_ClassNode> ParentNode;
	TArray<TSharedPtr<FEventFlowEditor_ClassNode> > SubNodes;

	void AddUniqueSubNode(TSharedPtr<FEventFlowEditor_ClassNode> SubNode);
};

struct FEventFlowSystem_Editor_ClassHelper
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnPackageListUpdated);

	FEventFlowSystem_Editor_ClassHelper(UClass* InRootClass);
	~FEventFlowSystem_Editor_ClassHelper();

	void GatherClasses(const UClass* BaseClass, TArray<FEventFlowSystem_Editor_ClassData>& AvailableClasses);
	static FString GetDeprecationMessage(const UClass* Class);

	void OnAssetAdded(const struct FAssetData& AssetData);
	void OnAssetRemoved(const struct FAssetData& AssetData);
	void InvalidateCache();
	void OnHotReload(bool bWasTriggeredAutomatically);

	static void AddUnknownClass(const FEventFlowSystem_Editor_ClassData& ClassData);
	static bool IsClassKnown(const FEventFlowSystem_Editor_ClassData& ClassData);
	static FOnPackageListUpdated OnPackageListUpdated;

	static int32 GetObservedBlueprintClassCount(UClass* BaseNativeClass);
	static void AddObservedBlueprintClasses(UClass* BaseNativeClass);
	void UpdateAvailableBlueprintClasses();

private:

	UClass* RootNodeClass;
	TSharedPtr<FEventFlowEditor_ClassNode> RootNode;
	static TArray<FName> UnknownPackages;
	static TMap<UClass*, int32> BlueprintClassCount;

	TSharedPtr<FEventFlowEditor_ClassNode> CreateClassDataNode(const struct FAssetData& AssetData);
	TSharedPtr<FEventFlowEditor_ClassNode> FindBaseClassNode(TSharedPtr<FEventFlowEditor_ClassNode> Node, const FString& ClassName);
	void FindAllSubClasses(TSharedPtr<FEventFlowEditor_ClassNode> Node, TArray<FEventFlowSystem_Editor_ClassData>& AvailableClasses);

	UClass* FindAssetClass(const FString& GeneratedClassPackage, const FString& AssetName);
	void BuildClassGraph();
	void AddClassGraphChildren(TSharedPtr<FEventFlowEditor_ClassNode> Node, TArray<TSharedPtr<FEventFlowEditor_ClassNode> >& NodeList);

	bool IsHidingClass(UClass* Class);
	bool IsHidingParentClass(UClass* Class);
	bool IsPackageSaved(FName PackageName);
};