// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FANTASYGAME_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	// Read / Write in the blueprint

public:
	// Constructor
	UPlayerAnimInstance();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Animation")
	float AimAnimationValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Animation")
	float DeltaTimeX;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
