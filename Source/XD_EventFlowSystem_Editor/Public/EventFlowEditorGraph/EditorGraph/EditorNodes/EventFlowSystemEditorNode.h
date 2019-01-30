// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "EventFlowGraphNodeBase.h"
#include "SubclassOf.h"
#include "EventFlowSystemEditorNode.generated.h"

class UEventFlowSequence_Branch;
class UEventFlowSequence_List;
class UXD_EventFlowSequenceBase;
class UXD_EventFlowElementBase;
class UEventSequenceEdNodeBase;
class FCompilerResultsLog;
class UEventSequenceBranch_SelectionEdNode;
class UEventFlowGraphBlueprintGeneratedClass;

/**
 * 
 */
UCLASS()
class UEventFlowSystemEditorNodeBase : public UEdGraphNode
{
	GENERATED_BODY()
public:
	UEventFlowSystemEditorNodeBase(const FObjectInitializer& ObjectInitializer);

	// Inherited via EdGraphNode.h
	TSharedPtr<SGraphNode> CreateVisualWidget() override; 	/** Create a visual widget to represent this node in a graph editor or graph panel.  If not implemented, the default node factory will be used. */
	void AllocateDefaultPins() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void PrepareForCopying() override;
	void DestroyNode() override;
	void AutowireNewNode(UEdGraphPin* FromPin) override;
	void GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const override;

	template<typename T>
	TArray<T*> GetChildNodes() const
	{
		TArray<T*> Res;
		for (UEdGraphPin* Pin : Pins)
		{
			if (Pin && Pin->Direction == EGPD_Output)
			{
				for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
				{
					if (T* Node = Cast<T>(LinkedPin->GetOwningNode()))
					{
						Res.Add(Node);
					}
				}
			}
		}
		return Res;
	}

	template<typename T>
	T* DuplicatedBpNode(UObject* Outer) const
	{
		T* InstanceNode = CastChecked<T>(StaticDuplicateObject(EventFlowBpNode, Outer, *EventFlowBpNode->GetName(), RF_Transactional));
		InstanceNode->SetFlags(RF_ArchetypeObject | RF_DefaultSubObject);
		return InstanceNode;
	}
public:
	virtual void SetAssetNode(UEventFlowGraphNodeBase* InNode);
	virtual UEventFlowGraphNodeBase* GetAssetNode();
	virtual void PostCopyNode();

	virtual TSharedPtr<SWidget> GetContentWidget();
	virtual void UpdateVisualNode();
	virtual FPinConnectionResponse CanLinkedTo(const UEventFlowSystemEditorNodeBase* AnotherNode) const;

	virtual bool GetNodeLinkableContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const;
protected:
	virtual bool HasOutputPins();
	virtual bool HasInputPins();
	TSharedPtr<SGraphNode> SlateNode;
public:
	UPROPERTY(Instanced)
	UEventFlowGraphNodeBase* EventFlowBpNode = nullptr;
};

UCLASS()
class UEventFlowSystemStartEdNode : public UEventFlowSystemEditorNodeBase
{
	GENERATED_BODY()
public:
	bool CanUserDeleteNode() const override { return false; }
	bool CanDuplicateNode() const override { return false; }

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	bool HasInputPins() override { return false; }
};

USTRUCT()
struct FNewElement_SchemaAction : public FEdGraphSchemaAction
{
public:
	GENERATED_USTRUCT_BODY()

	FNewElement_SchemaAction() :FEdGraphSchemaAction(), NewElementClass(nullptr) {}
	FNewElement_SchemaAction(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, UEventSequenceEdNodeBase* SequenceNode, TSubclassOf<UXD_EventFlowElementBase> NewElementClass) :FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), SequenceNode(SequenceNode), NewElementClass(NewElementClass) {}

	/** Execute this action, given the graph and schema, and possibly a pin that we were dragged from. Returns a node that was created by this action (if any). */
	UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;

	UPROPERTY()
	UEventSequenceEdNodeBase* SequenceNode;
	TSubclassOf<UXD_EventFlowElementBase> NewElementClass;
};

USTRUCT()
struct FNewBranch_SchemaAction : public FEdGraphSchemaAction
{
public:
	GENERATED_USTRUCT_BODY()

	FNewBranch_SchemaAction() :FEdGraphSchemaAction(), NewElementClass(nullptr) {}
	FNewBranch_SchemaAction(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, UEventSequenceEdNodeBase* SequenceEdNode, TSubclassOf<UXD_EventFlowElementBase> NewElementClass)
		:FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), SequenceEdNode(SequenceEdNode), NewElementClass(NewElementClass) {}

	/** Execute this action, given the graph and schema, and possibly a pin that we were dragged from. Returns a node that was created by this action (if any). */
	UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;

	UPROPERTY()
	UEventSequenceEdNodeBase* SequenceEdNode;

	TSubclassOf<UXD_EventFlowElementBase> NewElementClass;
};

UCLASS()
class UEventElementEdNode : public UEventFlowSystemEditorNodeBase
{
	GENERATED_BODY()
public:
	bool HasInputPins() override { return false; }
	bool HasOutputPins() override { return false; }

	UPROPERTY()
	UEventSequenceEdNodeBase* ParentNode;

	void DestroyNode() override;
};

UCLASS(abstract)
class UEventSequenceEdNodeBase : public UEventFlowSystemEditorNodeBase
{
	GENERATED_BODY()
public:
	void GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const override;

	void AddElement(UEventElementEdNode* Element);
	void RemoveElement(UEventElementEdNode* Element);

	virtual UXD_EventFlowSequenceBase* BuildSequenceTree(UEventFlowGraphBlueprintGeneratedClass* Outer, FCompilerResultsLog& MessageLog) const;

	static TSubclassOf<UEventSequenceEdNodeBase> GetEdNodeClassByRuntimeClass(const TSubclassOf<UXD_EventFlowSequenceBase>& RunTimeSequence);
public:
	UPROPERTY()
	TArray<UEventElementEdNode*> EventElements;
};

UCLASS()
class UEventSequenceListEdNode : public UEventSequenceEdNodeBase
{
	GENERATED_BODY()
public:
	UXD_EventFlowSequenceBase* BuildSequenceTree(UEventFlowGraphBlueprintGeneratedClass* Outer, FCompilerResultsLog& MessageLog) const override;
};

UCLASS()
class UEventSequenceBranchEdNode : public UEventSequenceEdNodeBase
{
	GENERATED_BODY()
public:
	FPinConnectionResponse CanLinkedTo(const UEventFlowSystemEditorNodeBase* AnotherNode) const override;
	bool GetNodeLinkableContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	UXD_EventFlowSequenceBase* BuildSequenceTree(UEventFlowGraphBlueprintGeneratedClass* Outer, FCompilerResultsLog& MessageLog) const override;
};

UCLASS()
class UEventSequenceBranch_SelectionEdNode : public UEventFlowSystemEditorNodeBase
{
	GENERATED_BODY()
public:
	FPinConnectionResponse CanLinkedTo(const UEventFlowSystemEditorNodeBase* AnotherNode) const override;
};
