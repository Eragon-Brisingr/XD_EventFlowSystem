// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemEditor.h"
#include "EventFlowSystem_Editor_Log.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "Slate.h"
#include "BlueprintEditorUtils.h"
#include "GraphEditorActions.h"
#include "EventFlowSystemEditorGraph.h"
#include "EventFlowSystemEditorGraphSchema.h"
#include "EdGraphUtilities.h"
#include "EventFlowSystemEditorNode.h"
#include "Editor.h"
#include "EventFlowDesignerApplicationMode.h"
#include "EventFlowSystemApplicationMode.h"


#define LOCTEXT_NAMESPACE "XD_EventFlowSystem_Editor"

FEventFlowSystemEditor::FEventFlowSystemEditor()
{

}

FEventFlowSystemEditor::~FEventFlowSystemEditor()
{
	UEventFlowGraphBlueprint* EditorGraph_Blueprint = GetTemplateBlueprintObj();
	if (EditorGraph_Blueprint)
	{
		EditorGraph_Blueprint->OnCompiled().RemoveAll(this);
	}
}

FLinearColor FEventFlowSystemEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FName FEventFlowSystemEditor::GetToolkitFName() const
{
	return FName("Graph Editor Template");
}

FText FEventFlowSystemEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Graph Editor Template");
}

FString FEventFlowSystemEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Graph").ToString();
}

void FEventFlowSystemEditor::InitEventFlowSystemGarphEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UEventFlowGraphBlueprint* InBP)
{
	EventFlowGraph = InBP->EventFlowGraph;

	if (EventFlowGraph->EdGraph == nullptr)
	{
		EventFlowSystem_Log("Creating a new graph.");
		EventFlowGraph->EdGraph = CastChecked<UEventFlowSystemEditorGraph>(FBlueprintEditorUtils::CreateNewGraph(EventFlowGraph, NAME_None, UEventFlowSystemEditorGraph::StaticClass(), UEventFlowSystemEditorGraphSchema::StaticClass()));
		EventFlowGraph->EdGraph->bAllowDeletion = false;

		//Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = EventFlowGraph->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*EventFlowGraph->EdGraph);
	}

// 	FGenericCommands::Register();
// 	FGraphEditorCommands::Register();
	InitBlueprintEditor(InMode, InToolkitHost, { InBP }, true);

	UpdatePreviewActor(GetBlueprintObj(), true);

	UEventFlowGraphBlueprint* EditorGraph_Blueprint = GetTemplateBlueprintObj();
	if (EditorGraph_Blueprint)
	{
		EditorGraph_Blueprint->OnCompiled().AddRaw(this, &FEventFlowSystemEditor::BlueprintCompiled);
	}
}

void FEventFlowSystemEditor::BlueprintCompiled(class UBlueprint* Blueprint)
{
	if (EventFlowGraph)
	{
		if (UEventFlowSystemEditorGraph* EdGraph = Cast<UEventFlowSystemEditorGraph>(EventFlowGraph->EdGraph))
		{
			EdGraph->BuildGraph();
			EdGraph->RefreshNodes();
		}
	}
}

void FEventFlowSystemEditor::InitalizeExtenders()
{
	FBlueprintEditor::InitalizeExtenders();
}

void FEventFlowSystemEditor::RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated /*= false*/)
{
	TSharedPtr<FEventFlowSystemEditor> ThisPtr(SharedThis(this));

	// Create the modes and activate one (which will populate with a real layout)
	TArray<TSharedRef<FApplicationMode>> TempModeList;

	TSharedRef<FEventFlowDesignerApplicationMode> DesignerApplicationMode = MakeShareable(new FEventFlowDesignerApplicationMode(ThisPtr));
	TSharedRef<FEventFlowSystemApplicationMode> FGraphApplicationMode = MakeShareable(new FEventFlowSystemApplicationMode(ThisPtr));

 	AddApplicationMode(DesignerApplicationMode->GetModeName(), DesignerApplicationMode);
 	AddApplicationMode(FGraphApplicationMode->GetModeName(), FGraphApplicationMode);
 
 	SetCurrentMode(DesignerApplicationMode->GetModeName());
}

FGraphAppearanceInfo FEventFlowSystemEditor::GetGraphAppearance(class UEdGraph* InGraph) const
{
	FGraphAppearanceInfo AppearanceInfo = FBlueprintEditor::GetGraphAppearance(InGraph);

	return AppearanceInfo;
}

void FEventFlowSystemEditor::AppendExtraCompilerResults(TSharedPtr<class IMessageLogListing> ResultsListing)
{
	FBlueprintEditor::AppendExtraCompilerResults(ResultsListing);
}

TSubclassOf<UEdGraphSchema> FEventFlowSystemEditor::GetDefaultSchemaClass() const
{
	return UEventFlowSystemEditorGraphSchema::StaticClass();
}

class UEventFlowGraphBlueprint* FEventFlowSystemEditor::GetTemplateBlueprintObj() const
{
	return Cast<UEventFlowGraphBlueprint>(GetBlueprintObj());
}

UEventFlowSystemEditorGraph* FEventFlowSystemEditor::GetEditorGraph() const
{
	return Cast<UEventFlowSystemEditorGraph>(EventFlowGraph->EdGraph);
}

void FEventFlowSystemEditor::SaveAsset_Execute()
{
	if (EventFlowGraph)
	{
		if (UEventFlowSystemEditorGraph* EdGraph = Cast<UEventFlowSystemEditorGraph>(EventFlowGraph->EdGraph))
		{
			EdGraph->BuildGraph();
		}
	}
	FBlueprintEditor::SaveAsset_Execute();
}

void FEventFlowSystemEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
{
	FBlueprintEditor::RegisterTabSpawners(TabManager);
}

void FEventFlowSystemEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
{
	FBlueprintEditor::UnregisterTabSpawners(TabManager);
}

// END Commands and binding
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE