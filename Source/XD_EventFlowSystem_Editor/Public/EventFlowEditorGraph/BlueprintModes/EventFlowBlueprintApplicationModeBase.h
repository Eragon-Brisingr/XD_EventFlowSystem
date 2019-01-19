// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintEditorModes.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

/**
 * 
 */
struct XD_EVENTFLOWSYSTEM_EDITOR_API FBlueprintApplicationModesTemplate
{
	// Mode constants
	static const FName DesignerMode;
	static const FName GraphMode;

	static FText GetLocalizedMode(const FName InMode);

private:
	FBlueprintApplicationModesTemplate() {}
};

class XD_EVENTFLOWSYSTEM_EDITOR_API FEventFlowBlueprintApplicationModeBase : public FBlueprintEditorApplicationMode
{
public:
	FEventFlowBlueprintApplicationModeBase(TSharedPtr<class FEventFlowSystemEditor> GraphEditorToolkit, FName InModeName);

	void AddModeSwitchToolBarExtension();

protected:
	UEventFlowGraphBlueprint* GetBlueprint() const;

	class FEventFlowSystemEditor* GetBlueprintEditor() const;

	TWeakPtr<class FEventFlowSystemEditor> EventFlowEditor;

	// Set of spawnable tabs in the mode
	FWorkflowAllowedTabSet TabFactories;
};
