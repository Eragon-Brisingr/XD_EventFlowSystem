// Fill out your copyright notice in the Description page of Project Settings.
#include "SEventFlowSystemGraphNode.h"
#include "Slate.h"
#include "EventFlowSystemEditorNode.h"
#include "SGraphPin.h"
#include "SEventFlowSystemGraphPin.h"
#include "EventFlowSystem_Editor_Log.h"
#include "SCommentBubble.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

void SEventFlowSystemGraphNode::Construct(const FArguments & InArgs, UEdGraphNode * InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SEventFlowSystemGraphNode::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	TSharedPtr<SErrorText> ErrorText;

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(FMargin(1.f, 5.0f))
			.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(LeftNodeBox, SVerticalBox)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SBorder)
                    .BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
                    .Padding(FMargin(5.0f))
                    .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.4f))
                    [
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(this, &SEventFlowSystemGraphNode::GetBP_NodeName)
						]
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.AutoHeight()
						[
							SAssignNew(InlineEditableText, SInlineEditableTextBlock)
							.Style(FEditorStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
							.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
							.IsReadOnly(this, &SEventFlowSystemGraphNode::IsNameReadOnly)
							.OnTextCommitted(this, &SEventFlowSystemGraphNode::OnNameTextCommited)
							.OnVerifyTextChanged(this, &SEventFlowSystemGraphNode::OnVerifyNameTextChanged)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							NodeTitle.ToSharedRef()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(ContentWidget, SBox)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							// 错误提示
							SAssignNew(ErrorText, SErrorText)
							.BackgroundColor(this, &SEventFlowSystemGraphNode::GetErrorColor)
							.ToolTipText(this, &SEventFlowSystemGraphNode::GetErrorMsgToolTip)
						]
                    ]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]
			]
		];

	//注释用的气泡
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	UpdateErrorInfo();
	ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);

	CreatePinWidgets();
	CreateContent();
}

void SEventFlowSystemGraphNode::CreatePinWidgets()
{
	UEventFlowSystemEditorNode* EdNode = CastChecked<UEventFlowSystemEditorNode>(GraphNode);
	for (int32 i = 0; i < EdNode->Pins.Num(); ++i)
	{
		UEdGraphPin* Pin = EdNode->Pins[i];
		if (!Pin->bHidden)
		{
			TSharedPtr<SGraphPin>NewPin = SNew(SEventFlowSystemGraphPin, Pin);
			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SEventFlowSystemGraphNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(0.f, 0.0f)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
	else
	{
		RightNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(0.f, 0.0f)
			[
				PinToAdd
			];
		OutputPins.Add(PinToAdd);
	}
}

bool SEventFlowSystemGraphNode::IsNameReadOnly() const
{
	return true;
}

void SEventFlowSystemGraphNode::OnNameTextCommited(const FText & InText, ETextCommit::Type CommitInfo)
{
	UEventFlowSystemEditorNode* UEdNode = CastChecked<UEventFlowSystemEditorNode>(GraphNode);

}

void SEventFlowSystemGraphNode::CreateContent()
{
	UEventFlowSystemEditorNode* Node = Cast<UEventFlowSystemEditorNode>(GraphNode);

	ContentWidget->SetContent(Node->GetContentWidget().ToSharedRef());
	ContentWidget->SetMinDesiredWidth(200.f);
}

FText SEventFlowSystemGraphNode::GetBP_NodeName() const
{
	UEventFlowSystemEditorNode* Node = Cast<UEventFlowSystemEditorNode>(GraphNode);
	if (Node->EventFlowBP_Node)
	{
		return Node->EventFlowBP_Node->GetNodeTitle();
	}
	return FText::GetEmpty();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE