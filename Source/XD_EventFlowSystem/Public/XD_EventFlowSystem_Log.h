// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "XD_EventFlowSystemUtility.generated.h"

/**
 * 
 */
DECLARE_LOG_CATEGORY_EXTERN(XD_EventFlowSystem_Log, Log, All);

#define EventFlowSystem_Display_Log(FMT, ...) UE_LOG(XD_EventFlowSystem_Log, Display, TEXT(FMT), ##__VA_ARGS__)
#define EventFlowSystem_Warning_Log(FMT, ...) UE_LOG(XD_EventFlowSystem_Log, Warning, TEXT(FMT), ##__VA_ARGS__)
#define EventFlowSystem_Error_Log(FMT, ...) UE_LOG(XD_EventFlowSystem_Log, Error, TEXT(FMT), ##__VA_ARGS__)