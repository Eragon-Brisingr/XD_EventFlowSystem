// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include <BlueprintEditor.h>
#include <Misc/NotifyHook.h>
#include <GraphEditor.h>
#include <IDetailsView.h>

class UEventFlowGraphBlueprint;
class UEventFlowSystemEditorGraph;

/**
 * 
 */
class FEventFlowSystemEditor : public FBlueprintEditor
{
public:
	FEventFlowSystemEditor();
	~FEventFlowSystemEditor();

	// Inherited via FAssetEditorToolkit
	FLinearColor GetWorldCentricTabColorScale() const override;
	FName GetToolkitFName() const override;
	FText GetBaseToolkitName() const override;
	FString GetWorldCentricTabPrefix() const override;
	void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	void SaveAsset_Execute() override;

	// Begin FBlueprintEditor
	void InitalizeExtenders() override;
	void RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated = false) override;
	FGraphAppearanceInfo GetGraphAppearance(class UEdGraph* InGraph) const override;
	void AppendExtraCompilerResults(TSharedPtr<class IMessageLogListing> ResultsListing) override;
	TSubclassOf<UEdGraphSchema> GetDefaultSchemaClass() const override;
	// End FBlueprintEditor

public:
	void InitEventFlowSystemGarphEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UEventFlowGraphBlueprint* EventFlowGraphBlueprint);
	void BlueprintCompiled(class UBlueprint* Blueprint);

	UEventFlowGraphBlueprint* GetEventFlowBlueprint() const;

	UEventFlowSystemEditorGraph* GetEventFlowGraph() const;
};
