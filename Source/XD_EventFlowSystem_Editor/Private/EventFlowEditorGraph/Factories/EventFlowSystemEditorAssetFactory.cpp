// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemEditorAssetFactory.h"
#include "EventFlowGraphBlueprint.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"
#include "KismetEditorUtilities.h"
#include "XD_EventFlowBase.h"

UEventFlowSystemEditorAssetFactory::UEventFlowSystemEditorAssetFactory()
{
	SupportedClass = UEventFlowGraphBlueprint::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UEventFlowSystemEditorAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UEventFlowGraphBlueprint* NewBP = CastChecked<UEventFlowGraphBlueprint>(FKismetEditorUtilities::CreateBlueprint(UXD_EventFlowBase::StaticClass(), InParent, Name, EBlueprintType::BPTYPE_Normal, UEventFlowGraphBlueprint::StaticClass(), UEventFlowGraphBlueprintGeneratedClass::StaticClass(), CallingContext));
	return NewBP;
}

UObject* UEventFlowSystemEditorAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn, NAME_None);
}
