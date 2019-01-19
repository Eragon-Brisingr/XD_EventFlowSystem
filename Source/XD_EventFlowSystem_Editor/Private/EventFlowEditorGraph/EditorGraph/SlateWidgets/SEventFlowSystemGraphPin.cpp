// Fill out your copyright notice in the Description page of Project Settings.
#include "SEventFlowSystemGraphPin.h"
#include "EventFlowSystem_EditorStyle.h"


void SEventFlowSystemGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	this->SetCursor(EMouseCursor::Default);

	bShowLabel = true;

	GraphPinObj = InPin;
	check(GraphPinObj != nullptr);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &SEventFlowSystemGraphPin::GetPinBorder)
		.BorderBackgroundColor(this, &SEventFlowSystemGraphPin::GetPinColor)
		.OnMouseButtonDown(this, &SEventFlowSystemGraphPin::OnPinMouseDown)
		.Cursor(this, &SEventFlowSystemGraphPin::GetPinCursor)
		.Padding(FMargin(10.0f))
	);
}

FSlateColor SEventFlowSystemGraphPin::GetPinColor() const
{
	return EventFlowSystem_EditorStyle::Pin::Default;
}

TSharedRef<SWidget> SEventFlowSystemGraphPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SEventFlowSystemGraphPin::GetPinBorder() const
{
	return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Body"));
}
