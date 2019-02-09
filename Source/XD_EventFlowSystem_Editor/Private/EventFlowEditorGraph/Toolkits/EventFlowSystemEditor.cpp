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
#include "EventFlowGraphBlueprint.h"


#define LOCTEXT_NAMESPACE "XD_EventFlowSystem_Editor"

FEventFlowSystemEditor::FEventFlowSystemEditor()
{

}

FEventFlowSystemEditor::~FEventFlowSystemEditor()
{
	UEventFlowGraphBlueprint* EditorGraph_Blueprint = GetEventFlowBlueprint();
	if (EditorGraph_Blueprint)
	{
		EditorGraph_Blueprint->OnCompiled().RemoveAll(this);
	}

	UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = CastChecked<UEventFlowSystemEditorGraph>(GetEventFlowBlueprint()->EdGraph);
	EventFlowSystemEditorGraph->OwingEditor = nullptr;
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

void FEventFlowSystemEditor::InitEventFlowSystemGarphEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UEventFlowGraphBlueprint* EventFlowGraphBlueprint)
{
	if (EventFlowGraphBlueprint->EdGraph == nullptr)
	{
		EventFlowSystem_Log("Creating a new graph.");
		UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = CastChecked<UEventFlowSystemEditorGraph>(FBlueprintEditorUtils::CreateNewGraph(EventFlowGraphBlueprint, NAME_None, UEventFlowSystemEditorGraph::StaticClass(), UEventFlowSystemEditorGraphSchema::StaticClass()));
		EventFlowGraphBlueprint->EdGraph = EventFlowSystemEditorGraph;
		EventFlowGraphBlueprint->EdGraph->bAllowDeletion = false;

		//Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = EventFlowGraphBlueprint->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*EventFlowGraphBlueprint->EdGraph);
	}

	UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = CastChecked<UEventFlowSystemEditorGraph>(EventFlowGraphBlueprint->EdGraph);
	EventFlowSystemEditorGraph->OwingEditor = this;

	InitBlueprintEditor(InMode, InToolkitHost, { EventFlowGraphBlueprint }, true);
	UpdatePreviewActor(GetBlueprintObj(), true);

	UEventFlowGraphBlueprint* EditorGraph_Blueprint = GetEventFlowBlueprint();
	if (EditorGraph_Blueprint)
	{
		EditorGraph_Blueprint->OnCompiled().AddRaw(this, &FEventFlowSystemEditor::BlueprintCompiled);
	}
}

void FEventFlowSystemEditor::BlueprintCompiled(class UBlueprint* Blueprint)
{
	if (GetEventFlowBlueprint())
	{
		if (UEventFlowSystemEditorGraph* EdGraph = Cast<UEventFlowSystemEditorGraph>(GetEventFlowBlueprint()->EdGraph))
		{
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

class UEventFlowGraphBlueprint* FEventFlowSystemEditor::GetEventFlowBlueprint() const
{
	return Cast<UEventFlowGraphBlueprint>(GetBlueprintObj());
}

UEventFlowSystemEditorGraph* FEventFlowSystemEditor::GetEditorGraph() const
{
	return Cast<UEventFlowSystemEditorGraph>(GetEventFlowBlueprint()->EdGraph);
}

void FEventFlowSystemEditor::SaveAsset_Execute()
{
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