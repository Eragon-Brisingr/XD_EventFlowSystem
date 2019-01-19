// Fill out your copyright notice in the Description page of Project Settings.

#include "EventFlowSystemApplicationMode.h"
#include "BlueprintEditorTabs.h"

FEventFlowSystemApplicationMode::FEventFlowSystemApplicationMode(TSharedPtr<class FEventFlowSystemEditor> GraphEditorToolkit)
	: FEventFlowBlueprintApplicationModeBase(GraphEditorToolkit, FBlueprintApplicationModesTemplate::GraphMode)
{
	TabLayout = FTabManager::NewLayout("EventFlowGraph_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.186721f)
				->SetHideTabWell(true)
				->AddTab(GraphEditorToolkit->GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.15f)
					->Split
					(
						FTabManager::NewStack()->SetSizeCoefficient(0.5f)
						->AddTab(FBlueprintEditorTabs::MyBlueprintID, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()->SetSizeCoefficient(0.5f)
						->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
					)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.70f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.80f)
						->AddTab("Document", ETabState::ClosedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.20f)
						->AddTab(FBlueprintEditorTabs::CompilerResultsID, ETabState::ClosedTab)
						->AddTab(FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab)
					)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.15f)
					->Split
					(
						FTabManager::NewStack()
						->AddTab(FBlueprintEditorTabs::PaletteID, ETabState::ClosedTab)
					)
				)
			)
		);

	AddModeSwitchToolBarExtension();
}

void FEventFlowSystemApplicationMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	FEventFlowBlueprintApplicationModeBase::RegisterTabFactories(InTabManager);
}

void FEventFlowSystemApplicationMode::PostActivateMode()
{
	FEventFlowBlueprintApplicationModeBase::PostActivateMode();
}
