// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <SCompoundWidget.h>
#include <NotifyHook.h>

class FEventFlowSystemEditor;

/**
 * 
 */
class XD_EVENTFLOWSYSTEM_EDITOR_API SEventFlowDetailsView : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SEventFlowDetailsView) {}
	SLATE_END_ARGS()
public:
	TSharedPtr<SEditableTextBox> NameTextBox;
	TSharedPtr<class IDetailsView> PropertyView;

	TWeakPtr<FEventFlowSystemEditor> Editor;

	void Construct(const FArguments& InArgs, TWeakPtr<FEventFlowSystemEditor> InEditor);

	EVisibility GetNameAreaVisibility() const { return EVisibility::Visible; }

	const FSlateBrush* GetNameIcon() const { return nullptr; }

	FText GetNameText() const;

	bool HandleVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage);

	void HandleNameTextChanged(const FText& Text);

	void HandleNameTextCommitted(const FText& Text, ETextCommit::Type CommitType);

	ECheckBoxState GetIsVariable() const;

	void HandleIsVariableChanged(ECheckBoxState CheckState);
};
