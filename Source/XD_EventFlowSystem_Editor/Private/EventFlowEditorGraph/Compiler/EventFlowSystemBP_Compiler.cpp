// Fill out your copyright notice in the Description page of Project Settings.

#include "EventFlowSystemBP_Compiler.h"
#include "EventFlowGraphBlueprint.h"
#include "EventFlowGraphBlueprintGeneratedClass.h"
#include "KismetReinstanceUtilities.h"
#include "EventFlowGraphNodeBase.h"
#include "EventFlowSystemEditorGraph.h"
#include "EventFlowSystemEditorNode.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowGraph"

#define CPF_Instanced (CPF_PersistentInstance | CPF_ExportObject | CPF_InstancedReference)

FEventFlowSystemBP_Compiler::FEventFlowSystemBP_Compiler(UEventFlowGraphBlueprint* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions, TArray<UObject*>* InObjLoaded)
	: FKismetCompilerContext(SourceSketch, InMessageLog, InCompilerOptions, InObjLoaded)
{

}

FEventFlowSystemBP_Compiler::~FEventFlowSystemBP_Compiler()
{
}

void FEventFlowSystemBP_Compiler::PreCompile()
{
	Super::PreCompile();

	UEventFlowGraphBlueprint* EditorGraph_Blueprint = GetGraphBlueprint();
	if (UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = Cast<UEventFlowSystemEditorGraph>(EditorGraph_Blueprint->EdGraph))
	{
		for (UEdGraphNode* EdNode : EventFlowSystemEditorGraph->GetAllRootLinkedNodes())
		{
			EdNode->ClearCompilerMessage();
		}
	}
}

void FEventFlowSystemBP_Compiler::SpawnNewClass(const FString& NewClassName)
{
	NewClass = FindObject<UEventFlowGraphBlueprintGeneratedClass>(Blueprint->GetOutermost(), *NewClassName);
	if (NewClass == NULL)
	{
		// If the class hasn't been found, then spawn a new one
		NewClass = NewObject<UEventFlowGraphBlueprintGeneratedClass>(Blueprint->GetOutermost(), FName(*NewClassName), RF_Public | RF_Transactional);
	}
	else
	{
		// Already existed, but wasn't linked in the Blueprint yet due to load ordering issues
		NewClass->ClassGeneratedBy = Blueprint;
		FBlueprintCompileReinstancer::Create(NewClass);
	}
}

void FEventFlowSystemBP_Compiler::CreateClassVariablesFromBlueprint()
{
	Super::CreateClassVariablesFromBlueprint();

	UEventFlowGraphBlueprint* EditorGraph_Blueprint = GetGraphBlueprint();

	if (UEventFlowSystemEditorGraph* EventFlowSystemEditorGraph = Cast<UEventFlowSystemEditorGraph>(EditorGraph_Blueprint->EdGraph))
	{
		for (UEdGraphNode* EdNode : EventFlowSystemEditorGraph->GetAllRootLinkedNodes())
		{
			UBlueprint::ForceLoad(EdNode);
			UBlueprint::ForceLoadMembers(EdNode);
			if (UEventFlowGraphNodeBase* Node = Cast<UEventFlowSystemEditorNodeBase>(EdNode)->EventFlowBpNode)
			{
				UBlueprint::ForceLoad(Node);
				UBlueprint::ForceLoadMembers(Node);
				if (Node->bIsVariable)
				{
					UProperty* NodeProperty = CreateVariable(*Node->GetVarRefName(), FEdGraphPinType(UEdGraphSchema_K2::PC_Object, NAME_None, Node->GetClass(), EPinContainerType::None, false, FEdGraphTerminalType()));
					if (NodeProperty)
					{
						NodeProperty->SetPropertyFlags(CPF_BlueprintVisible);
						NodeProperty->SetPropertyFlags(CPF_BlueprintReadOnly);
						NodeProperty->SetPropertyFlags(CPF_Instanced);
						NodeProperty->SetPropertyFlags(CPF_RepSkip);
						NodeProperty->SetMetaData(TEXT("Category"), TEXT("设计图表引用"));
					}
				}
			}
		}
	}
}

UEventFlowGraphBlueprint* FEventFlowSystemBP_Compiler::GetGraphBlueprint() const
{
	return CastChecked<UEventFlowGraphBlueprint>(Blueprint);
}

void FEventFlowSystemBP_Compiler::FinishCompilingClass(UClass* Class)
{
	UEventFlowGraphBlueprintGeneratedClass* BlueprintGeneratedClass = CastChecked<UEventFlowGraphBlueprintGeneratedClass>(Class);

	if (CompileOptions.CompileType != EKismetCompileType::SkeletonOnly)
	{
		UEventFlowGraphBlueprint* EventFlowBlueprint = CastChecked<UEventFlowGraphBlueprint>(Class->ClassGeneratedBy);
		if (UEventFlowSystemEditorGraph* DesignerGraph = Cast<UEventFlowSystemEditorGraph>(EventFlowBlueprint->EdGraph))
		{
			BlueprintGeneratedClass->SequenceList.Empty();
			BlueprintGeneratedClass->StartSequence = DesignerGraph->BuildSequenceTreeInstance(BlueprintGeneratedClass, MessageLog);
			EventFlowBlueprint->StartSequence = BlueprintGeneratedClass->StartSequence;
			if (BlueprintGeneratedClass->StartSequence == nullptr)
			{
				MessageLog.Error(TEXT("起始节点未配置任务序列"));
			}
		}

		BlueprintGeneratedClass->Bindings.Empty();

		for (int32 Idx = 0; Idx < EventFlowBlueprint->Bindings.Num(); ++Idx)
		{
			const FEventFlowDelegateEditorBinding& Binding = EventFlowBlueprint->Bindings[Idx];
			if (IsBindingValid(Binding, Class, EventFlowBlueprint, MessageLog))
			{
				if (DoesBindingTargetExist(Binding, EventFlowBlueprint))
				{
					BlueprintGeneratedClass->Bindings.Add(Binding.ToRuntimeBinding(EventFlowBlueprint));
					continue;
				}
			}

			EventFlowBlueprint->Bindings.RemoveAt(Idx--);
		}
	}

	FKismetCompilerContext::FinishCompilingClass(Class);
}

bool FEventFlowSystemBP_Compiler::IsBindingValid(const FEventFlowDelegateEditorBinding& Binding, UClass* Class, class UEventFlowGraphBlueprint* Blueprint, FCompilerResultsLog& MessageLog)
{
	if (!Binding.Object.IsValid())
	{
		return false;
	}

	if (UFunction* Function = Class->FindFunctionByName(Binding.GetFunctionName(Blueprint), EIncludeSuperFlag::IncludeSuper))
	{
		UDelegateProperty* DelegateProperty = FindField<UDelegateProperty>(Binding.Object->GetClass(), FName(*(Binding.PropertyName.ToString() + TEXT("Delegate"))));

		// Check the signatures to ensure these functions match.
		if (Function->IsSignatureCompatibleWith(DelegateProperty->SignatureFunction, UFunction::GetDefaultIgnoredSignatureCompatibilityFlags() | CPF_ReturnParm))
		{
			return true;
		}
		else
		{
			FText const ErrorFormat = LOCTEXT("BindingFunctionSigDontMatch", "Binding: property '@@' on widget '@@' bound to function '@@', but the sigatnures don't match.  The function must return the same type as the property and have no parameters.");
			MessageLog.Error(*ErrorFormat.ToString(), DelegateProperty, Binding.Object.Get(), Function);
		}
	}
	return false;
}

bool FEventFlowSystemBP_Compiler::DoesBindingTargetExist(const FEventFlowDelegateEditorBinding& Binding, class UEventFlowGraphBlueprint* Blueprint)
{
	UEventFlowSystemEditorGraph* Graph = Cast<UEventFlowSystemEditorGraph>(Blueprint->EdGraph);
	return Graph->GetAllRootLinkedNodes().ContainsByPredicate([&](const UEventFlowSystemEditorNodeBase* Node) {return Node && Node->EventFlowBpNode && Node->EventFlowBpNode == Binding.Object.Get(); });
}

#undef LOCTEXT_NAMESPACE