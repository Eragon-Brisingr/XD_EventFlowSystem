// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace EventFlowSystem_EditorStyle
{
	namespace Pin
	{
		const FLinearColor Default(0.02f, 0.02f, 0.02f);
	};

	namespace NodeBorder
	{
		const FLinearColor Default(0.2f, 0.2f, 0.2f, 0.2f);
		const FLinearColor Selected(1.00f, 0.08f, 0.08f);
	};

	namespace NodeBody
	{
		const FLinearColor Start(0.5f, 0.5f, 0.5f, 0.1f);
		const FLinearColor Sequence(0.1f, 0.1f, 0.1f);
		const FLinearColor Element(0.0f, 0.07f, 0.4f);
		const FLinearColor BranchSelection(0.4f, 0.1f, 0.0f);
	}

	namespace Debugger
	{
		const FLinearColor ActivedSequence(0.4f, 0.4f, 0.4f);
		const FLinearColor FinishedSequence(0.2f, 0.2f, 0.2f);

		const FLinearColor ActivedBranchSelection(1.6f, 0.4f, 0.0f);
		const FLinearColor FinishedBranchSelection(0.8f, 0.2f, 0.0f);

		const FLinearColor ActivedElement(0.0f, 0.28f, 1.6f);
		const FLinearColor FinishedElement(0.0f, 0.14f, 0.8f);
	}
};
