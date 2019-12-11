// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "XD_EventFlowSystem_Editor.h"
#include <Modules/ModuleManager.h>
#include "IAssetTools.h"
#include <AssetToolsModule.h>
#include "EventFlowEditorGraph/AssetTypeActions/EventFlowGraph_AssetTypeActions.h"
#include "EventFlowEditorGraph/Utility/EventFlowSystem_Editor_Log.h"
#include "EventFlowEditorGraph/Utility/EventFlowSystem_Editor_ClassHelper.h"
#include "EventFlowGraph/Nodes/EventFlowGraphNodeBase.h"
#include <KismetCompiler.h>
#include "EventFlowGraph/Blueprint/EventFlowGraphBlueprintGeneratedClass.h"
#include "EventFlowGraph/Blueprint/EventFlowGraphBlueprint.h"
#include "EventFlowEditorGraph/Compiler/EventFlowSystemBP_Compiler.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowGraph_EditorModule"

void FXD_EventFlowSystem_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedRef<IAssetTypeActions> Actions = MakeShareable(new FEventFlowGraph_AssetTypeActions());
	AssetTools.RegisterAssetTypeActions(Actions);

	FKismetCompilerContext::RegisterCompilerForBP(UEventFlowGraphBlueprint::StaticClass(), &FXD_EventFlowSystem_EditorModule::GetCompilerForBP);
}

void FXD_EventFlowSystem_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	NodeHelper.Reset();
}

void FXD_EventFlowSystem_EditorModule::StartNodeHelper()
{
	if (!NodeHelper.IsValid())
	{
		NodeHelper = MakeShareable(new FEventFlowSystem_Editor_ClassHelper(UEventFlowGraphNodeBase::StaticClass()));
		FEventFlowSystem_Editor_ClassHelper::AddObservedBlueprintClasses(UEventFlowGraphNodeBase::StaticClass());
	}
}

TSharedPtr<FKismetCompilerContext> FXD_EventFlowSystem_EditorModule::GetCompilerForBP(UBlueprint* BP, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompileOptions)
{
	return TSharedPtr<FKismetCompilerContext>(new FEventFlowSystemBP_Compiler(CastChecked<UEventFlowGraphBlueprint>(BP), InMessageLog, InCompileOptions));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FXD_EventFlowSystem_EditorModule, XD_EventFlowSystem_Editor)