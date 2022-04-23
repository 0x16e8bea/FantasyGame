// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LookMode.generated.h"

// Unreal look mode enum
UENUM(BlueprintType)
enum ELookMode
{
	LookMode_None UMETA(DisplayName = "None"),
	LookMode_Aiming UMETA(DisplayName = "Aiming"),
};