// Fill out your copyright notice in the Description page of Project Settings.


#include "TP_ThirdPerson/PlayerAnimInstance.h"

UPlayerAnimInstance::UPlayerAnimInstance() 
{


	AimAnimationValue = 0.0f;

}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	DeltaTimeX = DeltaSeconds;
	Super::NativeUpdateAnimation(DeltaSeconds);
}