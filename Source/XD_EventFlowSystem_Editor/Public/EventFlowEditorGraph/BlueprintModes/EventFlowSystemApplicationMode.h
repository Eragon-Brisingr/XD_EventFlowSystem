// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EventFlowEditorGraph/BlueprintModes/EventFlowBlueprintApplicationModeBase.h"

/**
 * 
 */
class XD_EVENTFLOWSYSTEM_EDITOR_API FEventFlowSystemApplicationMode : public FEventFlowBlueprintApplicationModeBase
{
public:
	FEventFlowSystemApplicationMode(TSharedPtr<class FEventFlowSystemEditor> GraphEditorToolkit);

	// FApplicationMode interface
	void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	void PostActivateMode() override;
	// End of FApplicationMode interface
};
