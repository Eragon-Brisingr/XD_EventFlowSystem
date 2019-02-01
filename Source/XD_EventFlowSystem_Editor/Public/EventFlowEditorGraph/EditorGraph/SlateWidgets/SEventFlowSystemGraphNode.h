// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "NotifyHook.h"
#include "IDetailsView.h"

/**
 * 
 */
class SEventFlowSystemGraphNode : public SGraphNode, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SEventFlowSystemGraphNode) {}
	SLATE_END_ARGS()

	// Inherited via SGraphNode
	void Construct(const FArguments& InArgs, UEdGraphNode* InNode);
	void UpdateGraphNode() override;
	void CreatePinWidgets() override;
	void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	bool IsNameReadOnly() const override;
	void SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel) override;
protected:
	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);
	FSlateColor GetNodeSelectedOutlineColor() const;
	FSlateColor GetNodeColor() const;
	FReply OnMouseDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent);
public:
	TSharedPtr<SBox> ContentWidget;

	TSharedPtr<SVerticalBox> ElementsBox;
	TArray<TSharedPtr<SGraphNode>> Elements;

	virtual void CreateContent();

	FText GetBP_NodeName() const;
};
