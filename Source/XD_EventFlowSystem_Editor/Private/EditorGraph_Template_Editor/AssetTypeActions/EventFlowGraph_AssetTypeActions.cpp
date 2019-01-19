// Fill out your copyright notice in the Description page of Project Settings.

#include "EventFlowGraph_AssetTypeActions.h"
#include "XD_EventFlowSystem_Editor.h"
#include "EventFlowSystemEditor.h"
#include "ModuleManager.h"
#include "EventFlowGraphBlueprint.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem_Editor"

FEventFlowGraph_AssetTypeActions::FEventFlowGraph_AssetTypeActions()
{
}

FText FEventFlowGraph_AssetTypeActions::GetName() const
{
	return LOCTEXT("GraphTypeActions", "事件流图表");
}

UClass * FEventFlowGraph_AssetTypeActions::GetSupportedClass() const
{
	return UEventFlowGraphBlueprint::StaticClass();
}

FColor FEventFlowGraph_AssetTypeActions::GetTypeColor() const
{
	return FColor::Red;
}

uint32 FEventFlowGraph_AssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Blueprint;
}

bool FEventFlowGraph_AssetTypeActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return false;
}

void FEventFlowGraph_AssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	FXD_EventFlowSystem_EditorModule& Helper= FModuleManager::LoadModuleChecked<FXD_EventFlowSystem_EditorModule>("XD_EventFlowSystem_Editor");
	Helper.StartNodeHelper();

	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto Object = InObjects.CreateConstIterator(); Object; Object++)
	{
		auto CustomBP = Cast<UEventFlowGraphBlueprint>(*Object);
		if (CustomBP != nullptr)
		{
			TSharedRef<FEventFlowSystemEditor> EditorToolkit = MakeShareable(new FEventFlowSystemEditor());
			EditorToolkit->InitEventFlowSystemGarphEditor(Mode, EditWithinLevelEditor, CustomBP);
		}
	}
}

#undef LOCTEXT_NAMESPACE