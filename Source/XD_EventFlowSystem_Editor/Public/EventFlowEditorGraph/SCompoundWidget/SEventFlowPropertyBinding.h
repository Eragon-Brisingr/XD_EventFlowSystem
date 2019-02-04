// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <SCompoundWidget.h>
#include <EdGraph/EdGraph.h>
#include <IDetailPropertyExtensionHandler.h>

class FEventFlowSystemEditor;
class IPropertyHandle;

/**
 * 
 */
class XD_EVENTFLOWSYSTEM_EDITOR_API SEventFlowPropertyBinding : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SEventFlowPropertyBinding)
	{}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, FEventFlowSystemEditor* InEditor, UDelegateProperty* DelegateProperty, TSharedRef<IPropertyHandle> Property);

	TSharedRef<SWidget> OnGenerateDelegateMenu(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle);

	const FSlateBrush* GetCurrentBindingImage(TSharedRef<IPropertyHandle> PropertyHandle) const;

	FText GetCurrentBindingText(TSharedRef<IPropertyHandle> PropertyHandle) const;

	EVisibility GetGotoBindingVisibility(TSharedRef<IPropertyHandle> PropertyHandle) const;

	FReply HandleGotoBindingClicked(TSharedRef<IPropertyHandle> PropertyHandle);

	bool CanRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle) const;

	void HandleRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle);

	void HandleCreateAndAddBinding(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle);

	void GotoFunction(UEdGraph* FunctionGraph);

private:
	FEventFlowSystemEditor* Editor;

	bool GeneratePureBindings = true;
	UFunction* BindableSignature;
};

class XD_EVENTFLOWSYSTEM_EDITOR_API FEventFlowDetailExtensionHandler : public IDetailPropertyExtensionHandler
{
public:
	FEventFlowDetailExtensionHandler(FEventFlowSystemEditor* BlueprintEditor)
		: BlueprintEditor(BlueprintEditor)
	{}

	virtual bool IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& InPropertyHandle) const override;

	virtual TSharedRef<SWidget> GenerateExtensionWidget(const UClass* InObjectClass, TSharedPtr<IPropertyHandle> InPropertyHandle) override;
private:
	FEventFlowSystemEditor* BlueprintEditor;
};
