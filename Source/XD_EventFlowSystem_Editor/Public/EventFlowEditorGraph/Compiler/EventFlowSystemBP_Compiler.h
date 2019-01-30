// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KismetCompiler.h"

class UEventFlowGraphBlueprint;
class FCompilerResultsLog;
struct FKismetCompilerOptions;

/**
 * 
 */
class FEventFlowSystemBP_Compiler : public FKismetCompilerContext
{
protected:
	typedef FKismetCompilerContext Super;

public:
	FEventFlowSystemBP_Compiler(UEventFlowGraphBlueprint* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions, TArray<UObject*>* InObjLoaded);
	~FEventFlowSystemBP_Compiler() override;

	// FKismetCompilerContext
	void SpawnNewClass(const FString& NewClassName) override;
	void CreateClassVariablesFromBlueprint() override;
	void FinishCompilingClass(UClass* Class) override;
	// End FKismetCompilerContext

protected:
	static bool IsBindingValid(const FEventFlowDelegateEditorBinding& Binding, UClass* Class, class UEventFlowGraphBlueprint* Blueprint, FCompilerResultsLog& MessageLog);
	static bool DoesBindingTargetExist(const FEventFlowDelegateEditorBinding& Binding, class UEventFlowGraphBlueprint* Blueprint);;

	UEventFlowGraphBlueprint* GetGraphBlueprint() const;
};
