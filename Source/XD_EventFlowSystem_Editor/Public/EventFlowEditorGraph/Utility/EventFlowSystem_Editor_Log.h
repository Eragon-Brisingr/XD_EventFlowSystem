// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
DECLARE_LOG_CATEGORY_EXTERN(Log_EventFlowSystem_Editor, Verbose, All);
#define EventFlowSystem_Log(Message,...) UE_LOG(Log_EventFlowSystem_Editor, Log, TEXT(Message), ##__VA_ARGS__)
#define EventFlowSystem_Display_Log(Message,...) UE_LOG(Log_EventFlowSystem_Editor, Display, TEXT(Message), ##__VA_ARGS__)
#define EventFlowSystem_Warning_Log(Message,...) UE_LOG(Log_EventFlowSystem_Editor, Warning, TEXT(Message), ##__VA_ARGS__)
#define EventFlowSystem_Error_Log(Message,...) UE_LOG(Log_EventFlowSystem_Editor, Error, TEXT(Message), ##__VA_ARGS__)


