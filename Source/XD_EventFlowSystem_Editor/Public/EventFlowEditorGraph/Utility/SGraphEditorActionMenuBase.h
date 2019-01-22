// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SBorder.h"
#include "GraphEditor.h"

class UEdGraph;
class UEdGraphPin;
class SGraphActionMenu;
struct FEdGraphSchemaAction;
struct FGraphActionListBuilderBase;

/**
 * 
 */
class XD_EVENTFLOWSYSTEM_EDITOR_API SGraphEditorActionMenuBase : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SGraphEditorActionMenuBase)
		: _GraphObj(static_cast<UEdGraph*>(NULL))
		, _GraphNode(nullptr)
		, _NewNodePosition(FVector2D::ZeroVector)
		, _AutoExpandActionMenu(false)
	{}

	SLATE_ARGUMENT(UEdGraph*, GraphObj)
		SLATE_ARGUMENT(UEdGraphNode*, GraphNode)
		SLATE_ARGUMENT(FVector2D, NewNodePosition)
		SLATE_ARGUMENT(TArray<UEdGraphPin*>, DraggedFromPins)
		SLATE_ARGUMENT(SGraphEditor::FActionMenuClosed, OnClosedCallback)
		SLATE_ARGUMENT(bool, AutoExpandActionMenu)
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);

	~SGraphEditorActionMenuBase();

	TSharedRef<SEditableTextBox> GetFilterTextBox();

protected:
	UEdGraph* GraphObj;
	UEdGraphNode* GraphNode;
	TArray<UEdGraphPin*> DraggedFromPins;
	FVector2D NewNodePosition;
	bool AutoExpandActionMenu;

	SGraphEditor::FActionMenuClosed OnClosedCallback;
	TSharedPtr<SGraphActionMenu> GraphActionMenu;

	virtual void OnActionSelected(const TArray< TSharedPtr<FEdGraphSchemaAction> >& SelectedAction, ESelectInfo::Type InSelectionType);

	/** Callback used to populate all actions list in SGraphActionMenu */
	virtual void CollectAllActions(FGraphActionListBuilderBase& OutAllActions) = 0;
};
