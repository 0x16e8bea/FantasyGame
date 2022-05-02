// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LookMode.h"
#include "UObject/Interface.h"
#include "CharacterDataInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCharacterDataInterface : public UInterface
{
	GENERATED_BODY()

	void GetCharacterInfo(FVector& NormalizedLookDirection, ELookMode& LookMode, bool& HasLookTarget);

	
};

/**
 * 
 */
class FANTASYGAME_API ICharacterDataInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Method that returns Velocity, Acceleration, Movement Input Vector, IsMoving, HasMovementInput, Speed, MovementInputAmount, Aiming Rotation, Aim Yaw Rate
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character Data")
	void GetCharacterInfo(FVector& NormalizedLookDirection, TEnumAsByte<ELookMode>& LookMode, bool& HasLookTarget);
	
	// FVector& Velocity, FVector& Acceleration, FVector& MovementInputVector, bool& IsMoving, bool& HasMovementInput, float& Speed, float& MovementInputAmount, FRotator& AimingRotation, float& AimYawRate
};
