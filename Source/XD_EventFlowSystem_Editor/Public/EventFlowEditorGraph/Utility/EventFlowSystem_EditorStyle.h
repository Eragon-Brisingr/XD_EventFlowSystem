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
		const FLinearColor BranchElement(0.4f, 0.07f, 0.0f);
	}
};
