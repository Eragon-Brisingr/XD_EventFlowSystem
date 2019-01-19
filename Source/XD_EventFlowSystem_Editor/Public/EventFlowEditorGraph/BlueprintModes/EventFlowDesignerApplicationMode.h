// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EventFlowBlueprintApplicationModeBase.h"

class SGraphEditor;

/**
 * 
 */
class XD_EVENTFLOWSYSTEM_EDITOR_API FEventFlowDesignerApplicationMode : public FEventFlowBlueprintApplicationModeBase
{
public:
	FEventFlowDesignerApplicationMode(TSharedPtr<class FEventFlowSystemEditor> GraphEditorToolkit);
	void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	void PreDeactivateMode() override;
	void PostActivateMode() override;

	static const FName DetailsTabId;
	static const FName GraphTabId;

public:
	TWeakPtr<SGraphEditor> DesignerGraphEditor;
	SGraphEditor* GetDesignerGraphEditor() const { return DesignerGraphEditor.Pin().Get(); }
	TWeakPtr<class SEventFlowDetailsView> DesignerDetails;

	TSharedPtr<FUICommandList> DesignerEditorCommands;

	SGraphEditor::FGraphEditorEvents DesignerGraphEvents;
private:
	void HandleSelectionChanged(const FGraphPanelSelectionSet& SelectionSet);
private:
	void BindDesignerToolkitCommands();

	FGraphPanelSelectionSet GetSelectedNodes();

	//Delegates
	void OnDesignerCommandSelectAllNodes();
	bool CanDesignerSelectAllNodes();

	void OnDesignerCommandCopy();
	bool CanDesignerCopyNodes();

	void OnDesignerCommandPaste();
	bool CanDesignerPasteNodes();

	void OnDesignerCommandCut();
	bool CanDesignerCutNodes();

	void OnDesignerCommandDuplicate();
	bool CanDesignerDuplicateNodes();

	void OnDesignerCommandDelete();
	bool CanDesignerDeleteNodes();
};
