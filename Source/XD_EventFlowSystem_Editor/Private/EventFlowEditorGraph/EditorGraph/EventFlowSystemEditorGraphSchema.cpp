// Fill out your copyright notice in the Description page of Project Settings.
#include "EventFlowSystemEditorGraphSchema.h"
#include "SubclassOf.h"
#include "EventFlowSystem_Editor_Log.h"
#include "EventFlowSystemEditorConnectionDrawingPolicy.h"
#include "UObjectIterator.h"
#include "EventFlowSystemGraph_NewNode_SchemaAction.h"
#include "EdGraph/EdGraph.h"
#include "EventFlowGraph.h"
#include "EventFlowSystemEditorNode.h"
#include "EventFlowSystem_Editor_ClassHelper.h"
#include "ModuleManager.h"
#include "XD_EventFlowSystem_Editor.h"

#define LOCTEXT_NAMESPACE "XD_EventFlowSystem"

UEventFlowSystemEditorGraphSchema::UEventFlowSystemEditorGraphSchema(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer){}

 void UEventFlowSystemEditorGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder & ContextMenuBuilder) const
 {
 
 	FText ToolTip = LOCTEXT("NewEditorGraphNodeTooltip", "Add a {NodeName} to the graph.");
 	FText MenuDesc = LOCTEXT("NewEditorGraphNodeDescription", "{NodeName}");
 
 	FXD_EventFlowSystem_EditorModule& Module = FModuleManager::LoadModuleChecked<FXD_EventFlowSystem_EditorModule>("XD_EventFlowSystem_Editor");
    TSharedPtr<FEventFlowSystem_Editor_ClassHelper> Helper = Module.GetHelper();
 	
 	//Gathering C++ classes
 
 	FCategorizedGraphActionListBuilder BaseBuilder(TEXT("C++ Defined Nodes"));
 
     TArray<FEventFlowSystem_Editor_ClassData> AllSubClasses;
     Helper->GatherClasses(UEventFlowGraphNodeBase::StaticClass(),AllSubClasses);
 
     for (auto& NativeClassData : AllSubClasses)
     {
         if (NativeClassData.GetClass()->HasAnyClassFlags(CLASS_Native))
         {
             FFormatNamedArguments Arguments;
             Arguments.Add(TEXT("NodeName"), NativeClassData.GetClass()->GetDisplayNameText());
             TSharedPtr<FEventFlowSystemGraph_NewNode_SchemaAction> NewNodeAction;
 
             NewNodeAction = MakeShareable(new FEventFlowSystemGraph_NewNode_SchemaAction(NativeClassData.GetCategory(), FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments), 0, NativeClassData.GetClass()));
 
             BaseBuilder.AddAction(NewNodeAction);
         }
     }
 
 	ContextMenuBuilder.Append(BaseBuilder);
 	
 	//Gathering child blueprints
     FCategorizedGraphActionListBuilder BlueprintBuilder(TEXT("Blueprint Defined Nodes"));
 
 	for (auto& BlueprintClassData : AllSubClasses)
 	{
 		if (!BlueprintClassData.GetClass()->HasAnyClassFlags(CLASS_Native))
 		{
 			FFormatNamedArguments Arguments;
 			Arguments.Add(TEXT("NodeName"), BlueprintClassData.GetClass()->GetDisplayNameText());
             TSharedPtr<FEventFlowSystemGraph_NewNode_SchemaAction> NewNodeAction;
 
             NewNodeAction = MakeShareable(new FEventFlowSystemGraph_NewNode_SchemaAction(BlueprintClassData.GetCategory(), FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments), 0, BlueprintClassData.GetClass()));
 
             BlueprintBuilder.AddAction(NewNodeAction);
 		}
 	}
 
 	ContextMenuBuilder.Append(BlueprintBuilder);
 }
 
 void UEventFlowSystemEditorGraphSchema::GetContextMenuActions(const UEdGraph* CurrentGraph, const UEdGraphNode* InGraphNode, const UEdGraphPin* InGraphPin, FMenuBuilder* MenuBuilder, bool bIsDebugging) const
 {
 	Super::GetContextMenuActions(CurrentGraph, InGraphNode, InGraphPin, MenuBuilder, bIsDebugging);
 }
 
 const FPinConnectionResponse UEventFlowSystemEditorGraphSchema::CanCreateConnection(const UEdGraphPin * A, const UEdGraphPin * B) const
 {
 	if(!(A && B))
 		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both pins must be available."));
 
 	if (A->GetOwningNode() == B->GetOwningNode())
 		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect a node to itself."));
 
 	if (A->Direction == EGPD_Input && B->Direction == EGPD_Input)
 		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect an input pin to another input pin."));
 
 	if (A->Direction == EGPD_Output && B->Direction == EGPD_Output)
 		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect an output pin to another output pin"));
 
 	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
 }
 
 FConnectionDrawingPolicy * UEventFlowSystemEditorGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect & InClippingRect, FSlateWindowElementList & InDrawElements, UEdGraph * InGraphObj) const
 {
 	return new FEventFlowSystemEditorConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
 }
 
 void UEventFlowSystemEditorGraphSchema::CreateDefaultNodesForGraph(UEdGraph & Graph) const
 {
 	if (Graph.Nodes.Num() == 0)
 	{
 		UEventFlowGraph* GraphAsset = Cast<UEventFlowGraph>(Graph.GetOuter());
 
 		GraphAsset->Modify();
 		Graph.Modify();
 		
 		UEventFlowGraphNodeBase* AssetNode = GraphAsset->SpawnNodeInsideGraph<UEventFlowGraphNodeBase>(UEventFlowGraphNodeBase::StaticClass());
 
 		FGraphNodeCreator<UEventFlowSystemEditorNode>Creator(Graph);
 		UEventFlowSystemEditorNode* EdNode = Creator.CreateNode();
 		EdNode->SetAssetNode(AssetNode);
 		EdNode->AllocateDefaultPins();
 
 		Creator.Finalize();
 
 		EdNode->NodePosX = 0;
 		EdNode->NodePosY = 0;
 	}
 }
#undef LOCTEXT_NAMESPACE