// Fill out your copyright notice in the Description page of Project Settings.
#include "SEventFlowSystemGraphNode.h"
#include "Slate.h"
#include "EventFlowSystemEditorNode.h"
#include "SGraphPin.h"
#include "SEventFlowSystemGraphPin.h"
#include "EventFlowSystem_Editor_Log.h"
#include "SCommentBubble.h"
#include "NodeFactory.h"
#include "SGraphNode.h"
#include "SGraphPanel.h"
#include "EventFlowSystem_EditorStyle.h"
#include <PropertyEditorModule.h>
#include <IDetailCustomization.h>
#include <DetailLayoutBuilder.h>

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
			.Padding(FMargin(1.f, 1.f))
			.BorderBackgroundColor(this, &SEventFlowSystemGraphNode::GetNodeSelectedOutlineColor)
			.OnMouseButtonDown(this, &SEventFlowSystemGraphNode::OnMouseDown)
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
                    .BorderBackgroundColor(this, &SEventFlowSystemGraphNode::GetNodeColor)
                    [
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(this, &SEventFlowSystemGraphNode::GetBP_NodeName)
					.Visibility_Lambda([this]() {return Cast<UEventFlowSystemEditorNodeBase>(GraphNode)->EventFlowBpNode ? EVisibility::Visible : EVisibility::Collapsed; })
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
						[
							SAssignNew(ElementsBox, SVerticalBox)
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

	UEventSequenceEdNodeBase* SequenceEdNode = Cast<UEventSequenceEdNodeBase>(GraphNode);
	if (SequenceEdNode)
	{
		for (int32 i = 0; i < SequenceEdNode->EventElements.Num(); i++)
		{
			if (SequenceEdNode->EventElements[i])
			{
				TSharedPtr<SGraphNode> DecoratorWidget = FNodeFactory::CreateNodeWidget(SequenceEdNode->EventElements[i]);

				if (OwnerGraphPanelPtr.IsValid())
				{
					DecoratorWidget->SetOwner(OwnerGraphPanelPtr.Pin().ToSharedRef());
				}

				ElementsBox->AddSlot()
					.AutoHeight()
					[
						DecoratorWidget.ToSharedRef()
					];
				Elements.Add(DecoratorWidget);

				DecoratorWidget->UpdateGraphNode();
			}
		}
	}
}

void SEventFlowSystemGraphNode::CreatePinWidgets()
{
	UEventFlowSystemEditorNodeBase* EdNode = CastChecked<UEventFlowSystemEditorNodeBase>(GraphNode);
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

void SEventFlowSystemGraphNode::SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel)
{
	SGraphNode::SetOwner(OwnerPanel);

	for (TSharedPtr<SGraphNode>& Element : Elements)
	{
		if (Element.IsValid())
		{
			Element->SetOwner(OwnerPanel);
			OwnerPanel->AttachGraphEvents(Element);
		}
	}
}

void SEventFlowSystemGraphNode::OnNameTextCommited(const FText & InText, ETextCommit::Type CommitInfo)
{
	UEventFlowSystemEditorNodeBase* UEdNode = CastChecked<UEventFlowSystemEditorNodeBase>(GraphNode);
}

FSlateColor SEventFlowSystemGraphNode::GetNodeSelectedOutlineColor() const
{
	if (UEventElementEdNode* EventElementEdNode = Cast<UEventElementEdNode>(GraphNode))
	{
		return GetOwnerPanel()->SelectionManager.SelectedNodes.Contains(GraphNode) ? EventFlowSystem_EditorStyle::NodeBorder::Selected : EventFlowSystem_EditorStyle::NodeBorder::Default;
	}
	else
	{
		return EventFlowSystem_EditorStyle::NodeBorder::Default;
	}
}

FSlateColor SEventFlowSystemGraphNode::GetNodeColor() const
{
	UEventFlowSystemEditorNodeBase* Node = Cast<UEventFlowSystemEditorNodeBase>(GraphNode);
	return Node->GetNodeColor();
}

FReply SEventFlowSystemGraphNode::OnMouseDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
{
	if (UEventElementEdNode* EventElementEdNode = Cast<UEventElementEdNode>(GraphNode))
	{
		GetOwnerPanel()->SelectionManager.ClickedOnNode(GraphNode, MouseEvent);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SEventFlowSystemGraphNode::CreateContent()
{
	ContentWidget->SetMinDesiredWidth(200.f);

	UEventFlowSystemEditorNodeBase* Node = Cast<UEventFlowSystemEditorNodeBase>(GraphNode);
	if (UEventFlowGraphNodeBase* EventFlowBpNode = Node->EventFlowBpNode)
	{
		FDetailsViewArgs DetailsViewArgs;
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.bLockable = false;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		TSharedRef<IDetailsView> PropertyView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
		//PropertyView->SetExtensionHandler(MakeShareable(new FEventFlowDetailExtensionHandler(Editor.Pin().Get())));
		class FEventFlowContentWidgetDetailCustomization : public IDetailCustomization
		{
		public:
			// IDetailCustomization interface
			virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
			{
				DetailBuilder.HideCategory(TEXT("高级"));
			}
		};
		PropertyView->RegisterInstancedCustomPropertyLayout(EventFlowBpNode->GetClass(), FOnGetDetailCustomizationInstance::CreateLambda([] {return MakeShareable(new FEventFlowContentWidgetDetailCustomization); }));
		PropertyView->SetObject(EventFlowBpNode);

		ContentWidget->SetContent(PropertyView);
	}
}

FText SEventFlowSystemGraphNode::GetBP_NodeName() const
{
	UEventFlowSystemEditorNodeBase* Node = Cast<UEventFlowSystemEditorNodeBase>(GraphNode);
	if (Node->EventFlowBpNode)
	{
		return Node->EventFlowBpNode->GetNodeTitle();
	}
	return FText::GetEmpty();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE