// Fill out your copyright notice in the Description page of Project Settings.

#include "EventFlowEditorGraph/BlueprintModes/EventFlowDesignerApplicationMode.h"
#include <WorkflowOrientedApp/WorkflowTabFactory.h>
#include <BlueprintEditor.h>
#include <PropertyEditorModule.h>
#include <GraphEditor.h>
#include <Framework/Commands/GenericCommands.h>
#include <EdGraphUtilities.h>
#include <HAL/PlatformApplicationMisc.h>
#include <SBlueprintEditorToolbar.h>
#include <Kismet2/BlueprintEditorUtils.h>
#include <PropertyEditorDelegates.h>
#include <SKismetInspector.h>
#include <Framework/MultiBox/MultiBoxBuilder.h>
#include <K2Node_ComponentBoundEvent.h>
#include <Framework/Application/SlateApplication.h>
#include <ScopedTransaction.h>
#include <BlueprintEditorTabs.h>

#include "EventFlowEditorGraph/Toolkits/EventFlowSystemEditor.h"
#include "EventFlowEditorGraph/EditorGraph/EditorNodes/EventFlowSystemEditorNode.h"
#include "EventFlowEditorGraph/EditorGraph/EventFlowSystemEditorGraph.h"
#include "EventFlowGraph/Blueprint/EventFlowGraphBlueprint.h"
#include "Events/XD_EventFlowElementBase.h"
#include "EventFlowEditorGraph/Utility/EventFlowSystem_Editor_Log.h"
#include "EventFlowEditorGraph/SCompoundWidget/SEventFlowDetailsView.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowGraph"

const FName FEventFlowDesignerApplicationMode::DetailsTabId(TEXT("EventFlowDesigner_DetailsTabId"));
const FName FEventFlowDesignerApplicationMode::GraphTabId(TEXT("EventFlowDesigner_GraphTabId"));

struct FFunctionInfo
{
	FFunctionInfo()
		: Function(nullptr)
	{
	}

	FText DisplayName;
	FString Tooltip;

	FName FuncName;
	UFunction* Function;
};

struct FEventFlowDesignerDetailsSummoner : public FWorkflowTabFactory
{
public:
	FEventFlowDesignerDetailsSummoner(class FEventFlowDesignerApplicationMode* DesignerApplicationMode, TSharedPtr<class FEventFlowSystemEditor> InDesignGraphEditor);

	FEventFlowDesignerApplicationMode* DesignerApplicationMode;
	TWeakPtr<class FEventFlowSystemEditor> InDesignGraphEditor;

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FEventFlowDesignerDetailsSummoner::FEventFlowDesignerDetailsSummoner(class FEventFlowDesignerApplicationMode* DesignerApplicationMode, TSharedPtr<class FEventFlowSystemEditor> InDesignGraphEditor)
	: FWorkflowTabFactory(FEventFlowDesignerApplicationMode::DetailsTabId, InDesignGraphEditor),
	DesignerApplicationMode(DesignerApplicationMode),
	InDesignGraphEditor(InDesignGraphEditor)
{
	TabLabel = LOCTEXT("EventFlowDesingerDetails_TabLabel", "细节");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("EventFlow_DesingerDetails_ViewMenu_Desc", "细节");
	ViewMenuTooltip = LOCTEXT("EventFlow_DesingerDetails_ViewMenu_ToolTip", "Show the Details");
}

TSharedRef<SWidget> FEventFlowDesignerDetailsSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedRef<SEventFlowDetailsView> DesignerDetails = SNew(SEventFlowDetailsView, InDesignGraphEditor);
	DesignerApplicationMode->DesignerDetails = DesignerDetails;
	return DesignerDetails;
}

struct FEventFlowDesignerGraphSummoner : public FWorkflowTabFactory
{
public:
	FEventFlowDesignerGraphSummoner(class FEventFlowDesignerApplicationMode* DesignerApplicationMode, TSharedPtr<class FEventFlowSystemEditor> InDesignGraphEditor);

	FEventFlowDesignerApplicationMode* DesignerApplicationMode;
	TWeakPtr<class FEventFlowSystemEditor> InDesignGraphEditor;

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FEventFlowDesignerGraphSummoner::FEventFlowDesignerGraphSummoner(class FEventFlowDesignerApplicationMode* DesignerApplicationMode, TSharedPtr<class FEventFlowSystemEditor> InDesignGraphEditor)
	:FWorkflowTabFactory(FEventFlowDesignerApplicationMode::GraphTabId, InDesignGraphEditor),
	DesignerApplicationMode(DesignerApplicationMode),
	InDesignGraphEditor(InDesignGraphEditor)
{
	TabLabel = LOCTEXT("EventFlowDesingerGraph_TabLabel", "图表");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "PhysicsAssetEditor.Tabs.Graph");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("EventFlowDesingerGraph_ViewMenu_Desc", "图表");
	ViewMenuTooltip = LOCTEXT("EventFlowDesingerGraph_ViewMenu_ToolTip", "Show the Garph");
}

TSharedRef<SWidget> FEventFlowDesignerGraphSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SAssignNew(DesignerApplicationMode->DesignerGraphEditor, SGraphEditor)
		.AdditionalCommands(DesignerApplicationMode->DesignerEditorCommands)
		.GraphToEdit(InDesignGraphEditor.Pin()->GetEventFlowBlueprint()->EdGraph)
		.GraphEvents(DesignerApplicationMode->DesignerGraphEvents);
}

FEventFlowDesignerApplicationMode::FEventFlowDesignerApplicationMode(TSharedPtr<class FEventFlowSystemEditor> GraphEditorToolkit)
	:FEventFlowBlueprintApplicationModeBase(GraphEditorToolkit, FBlueprintApplicationModesTemplate::DesignerMode),
	EventFlowDebugger(this)
{
	WorkspaceMenuCategory = FWorkspaceItem::NewGroup(LOCTEXT("EventFlowWorkspaceMenu_Designer", "EventFlowDesigner"));

	TabLayout = FTabManager::NewLayout("EventFlowDesigner_Layout_v1_1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.2f)
				->AddTab(GraphEditorToolkit->GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.8f)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetHideTabWell(true)
						->SetSizeCoefficient(0.8f)
						->AddTab(GraphTabId, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.2f)
						->AddTab(FBlueprintEditorTabs::CompilerResultsID, ETabState::OpenedTab)
						->AddTab(FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab)
					)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.2f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
			)
		);

 	TabFactories.RegisterFactory(MakeShareable(new FEventFlowDesignerDetailsSummoner(this, GraphEditorToolkit)));
 	TabFactories.RegisterFactory(MakeShareable(new FEventFlowDesignerGraphSummoner(this, GraphEditorToolkit)));

	ToolbarExtender = MakeShareable(new FExtender);

	if (UToolMenu* Toolbar = GraphEditorToolkit->RegisterModeToolbarIfUnregistered(GetModeName()))
	{
		GraphEditorToolkit->GetToolbarBuilder()->AddCompileToolbar(Toolbar);
		GraphEditorToolkit->GetToolbarBuilder()->AddDebuggingToolbar(Toolbar);
	}
	AddModeSwitchToolBarExtension();

	BindDesignerToolkitCommands();
	DesignerGraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateRaw(this, &FEventFlowDesignerApplicationMode::HandleSelectionChanged);
}

void FEventFlowDesignerApplicationMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	FEventFlowSystemEditor *BP = GetBlueprintEditor();

	BP->RegisterToolbarTab(InTabManager.ToSharedRef());
	BP->PushTabFactories(TabFactories);
	BP->PushTabFactories(BlueprintEditorTabFactories);
}

void FEventFlowDesignerApplicationMode::PreDeactivateMode()
{

}

void FEventFlowDesignerApplicationMode::PostActivateMode()
{

}

void FEventFlowDesignerApplicationMode::HandleSelectionChanged(const FGraphPanelSelectionSet& SelectionSet)
{
	if (DesignerDetails.IsValid())
	{
		TSharedPtr<SEventFlowDetailsView> Details = DesignerDetails.Pin();

		TArray<UObject*> ShowObjects;
		for (UObject* Obj : SelectionSet)
		{
			if (UEventFlowSystemEditorNodeBase* EventSystemEdGraphNode = Cast<UEventFlowSystemEditorNodeBase>(Obj))
			{
				if (EventSystemEdGraphNode->EventFlowBpNode)
				{
					ShowObjects.Add(EventSystemEdGraphNode->EventFlowBpNode);
				}
			}
		}

		Details->PropertyView->SetObjects(ShowObjects, true);
	}
}

void FEventFlowDesignerApplicationMode::BindDesignerToolkitCommands()
{
	if (!DesignerEditorCommands.IsValid())
	{
		DesignerEditorCommands = MakeShareable(new FUICommandList());

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandSelectAllNodes),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerSelectAllNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Cut,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandCut),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerCutNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Copy,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandCopy),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerCopyNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Paste,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandPaste),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerPasteNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandDuplicate),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerDuplicateNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::OnDesignerCommandDelete),
			FCanExecuteAction::CreateRaw(this, &FEventFlowDesignerApplicationMode::CanDesignerDeleteNodes)
		);

	}
}

FGraphPanelSelectionSet FEventFlowDesignerApplicationMode::GetSelectedNodes()
{
	return GetDesignerGraphEditor()->GetSelectedNodes();
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandSelectAllNodes()
{
	if (DesignerGraphEditor.IsValid())
	{
		GetDesignerGraphEditor()->SelectAllNodes();
	}
}

bool FEventFlowDesignerApplicationMode::CanDesignerSelectAllNodes()
{
	return true;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandCut()
{
 	const FScopedTransaction Transaction(FGenericCommands::Get().Cut->GetDescription());
 
 	OnDesignerCommandCopy();
 
 	const FGraphPanelSelectionSet OldSelectedNodes = GetDesignerGraphEditor()->GetSelectedNodes();
 	GetDesignerGraphEditor()->ClearSelectionSet();
 	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
 	{
 		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
 		if (Node && Node->CanDuplicateNode())
 		{
 			GetDesignerGraphEditor()->SetNodeSelection(Node, true);
 		}
 	}
 
 	OnDesignerCommandDelete();
 
 	GetDesignerGraphEditor()->ClearSelectionSet();
 
 	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
 	{
 		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
 		if (Node)
 		{
 			GetDesignerGraphEditor()->SetNodeSelection(Node, true);
 		}
 	}
}

bool FEventFlowDesignerApplicationMode::CanDesignerCutNodes()
{
	return false;

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*FGraphPanelSelectionSet::TConstIterator(SelectedNodes)))
		{
			return Node->CanDuplicateNode();
		}
	}
	return true;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandCopy()
{
 	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
 	FString ExportedText;
 
 	for (FGraphPanelSelectionSet::TIterator it(SelectedNodes); it; ++it)
 	{
 		UEdGraphNode* Node = Cast<UEdGraphNode>(*it);
 		if (Node)
 			Node->PrepareForCopying();
 		else
 			it.RemoveCurrent();
 	}
 
 	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
 	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
 
 	for (FGraphPanelSelectionSet::TIterator it(SelectedNodes); it; ++it)
 	{
 		UEventFlowSystemEditorNodeBase* Node = Cast<UEventFlowSystemEditorNodeBase>(*it);
 		if (Node)
 		{
 			Node->PostCopyNode();
 		}
 	}
}

bool FEventFlowDesignerApplicationMode::CanDesignerCopyNodes()
{
	return false;

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*FGraphPanelSelectionSet::TConstIterator(SelectedNodes)))
		{
			return Node->CanDuplicateNode();
		}
	}
	return true;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandPaste()
{
 	const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
 
 	const FVector2D PasteLocation = GetDesignerGraphEditor()->GetPasteLocation();
 
 	UEdGraph* EdGraph = GetDesignerGraphEditor()->GetCurrentGraph();
 	EdGraph->Modify();
 	GetDesignerGraphEditor()->ClearSelectionSet();
 
 	FString ExportedText;
 	FPlatformApplicationMisc::ClipboardPaste(ExportedText);
 	TSet<UEdGraphNode*> ImportedNodes;
 	FEdGraphUtilities::ImportNodesFromText(EdGraph, ExportedText, ImportedNodes);
 
 	FVector2D AvgNodePosition(0.0f, 0.0f);
 
 	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
 	{
 		UEdGraphNode* Node = *It;
 		AvgNodePosition.X += Node->NodePosX;
 		AvgNodePosition.Y += Node->NodePosY;
 	}
 
 	float InvNumNodes = 1.0f / float(ImportedNodes.Num());
 	AvgNodePosition.X *= InvNumNodes;
 	AvgNodePosition.Y *= InvNumNodes;
 
 	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
 	{
 		UEdGraphNode* Node = *It;
 		GetDesignerGraphEditor()->SetNodeSelection(Node, true);
 
 		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + PasteLocation.X;
 		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + PasteLocation.Y;
 
 		Node->SnapToGrid(16);
 
 		// Give new node a different Guid from the old one
 		Node->CreateNewGuid();
 	}
 
 	GetDesignerGraphEditor()->NotifyGraphChanged();
 
 	UObject* GraphOwner = EdGraph->GetOuter();
 	if (GraphOwner)
 	{
 		GraphOwner->PostEditChange();
 		GraphOwner->MarkPackageDirty();
 	}
}

bool FEventFlowDesignerApplicationMode::CanDesignerPasteNodes()
{
	return false;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandDuplicate()
{
	const FScopedTransaction Transaction(FGenericCommands::Get().Duplicate->GetDescription());
	OnDesignerCommandCopy();
	OnDesignerCommandPaste();
}

bool FEventFlowDesignerApplicationMode::CanDesignerDuplicateNodes()
{
	return false;

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*FGraphPanelSelectionSet::TConstIterator(SelectedNodes)))
		{
			return Node->CanDuplicateNode();
		}
	}
	return true;
}

void FEventFlowDesignerApplicationMode::OnDesignerCommandDelete()
{
	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());

	GetDesignerGraphEditor()->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	GetDesignerGraphEditor()->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(SelectedNodes); It; ++It)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*It))
		{
			if (Node->CanUserDeleteNode())
			{
				Node->Modify();
				Node->DestroyNode();
			}
		}
	}
}

bool FEventFlowDesignerApplicationMode::CanDesignerDeleteNodes()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*FGraphPanelSelectionSet::TConstIterator(SelectedNodes)))
		{
			return Node->CanUserDeleteNode();
		}
	}
	return true;
}

void FEventFlowDesignerApplicationMode::FEventFlowDebugger::Tick(float DeltaTime)
{
	for (TWeakObjectPtr<UEventFlowSystemEditorNodeBase> Node : PreActiveNodes)
	{
		if (Node.IsValid())
		{
			Node->DebugState = EEventFlowSystemEditorNodeDebugState::None;
		}
	}
	PreActiveNodes.Empty();

	if (UXD_EventFlowBase* WatchedEventFlow = Cast<UXD_EventFlowBase>(DesignerApplicationMode->GetBlueprint()->GetObjectBeingDebugged()))
	{
		if (UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = Cast<UEventFlowSystemEditorGraph>(DesignerApplicationMode->GetBlueprint()->EdGraph))
		{
			auto ChildNodes = EventFlowSystemEditorGraph->StartNode->GetChildNodes<UEventSequenceEdNodeBase>();
			if (ChildNodes.Num() > 0)
			{
				ChildNodes[0]->UpdateDebugInfo(WatchedEventFlow, 0, PreActiveNodes);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE