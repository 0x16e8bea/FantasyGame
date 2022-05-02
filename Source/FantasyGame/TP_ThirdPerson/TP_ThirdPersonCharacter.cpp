// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_ThirdPersonCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ATP_ThirdPersonCharacter

ATP_ThirdPersonCharacter::ATP_ThirdPersonCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create LookTarget
	LookTarget = CreateDefaultSubobject<USceneComponent>(TEXT("LookTarget"));
	LookTarget->SetupAttachment(RootComponent);
	LookTarget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	LookTarget->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	LookTarget->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

// Begin Play
void ATP_ThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Call UpdateLookTarget every frame
void ATP_ThirdPersonCharacter::Tick(float DeltaTime)
{
	UpdateLookLocation();
	Super::Tick(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATP_ThirdPersonCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);


	FInputActionBinding SprintPressed("Sprint", IE_Pressed);
	SprintPressed.ActionDelegate.GetDelegateForManualSet().BindLambda([this]()
	{
		IsSprinting = true;
		UpdateMovementSpeed();
	});

	PlayerInputComponent->AddActionBinding(SprintPressed);

	FInputActionBinding SprintReleased("Sprint", IE_Released);
	SprintReleased.ActionDelegate.GetDelegateForManualSet().BindLambda([this]()
	{
		IsSprinting = false;
		UpdateMovementSpeed();
	});

	PlayerInputComponent->AddActionBinding(SprintReleased);

	
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATP_ThirdPersonCharacter::ActivateAction);

	// Lamda to set aiming true (Finput action binding)
	FInputActionBinding AimPressed("Aim", IE_Pressed);
	AimPressed.ActionDelegate.GetDelegateForManualSet().BindLambda([this]()
	{
		SetAiming(true);
	});

	PlayerInputComponent->AddActionBinding(AimPressed);

	FInputActionBinding AimReleased("Aim", IE_Released);
	AimReleased.ActionDelegate.GetDelegateForManualSet().BindLambda([this]()
	{
		SetAiming(false);
	});

	PlayerInputComponent->AddActionBinding(AimReleased);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ATP_ThirdPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ATP_ThirdPersonCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ATP_ThirdPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ATP_ThirdPersonCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ATP_ThirdPersonCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ATP_ThirdPersonCharacter::TouchStopped);
}

void ATP_ThirdPersonCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ATP_ThirdPersonCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ATP_ThirdPersonCharacter::ActivateAction()
{
	// Print out bang in console
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Bang"));
}

void ATP_ThirdPersonCharacter::UpdateLookLocation()
{
	// If the look target is not null, update the look location.
	if (ActorToLookAt)
	{
		LookAtTarget = true;
		
		NormalizedLookDirection = (ActorToLookAt->GetActorLocation() - this->GetActorLocation());
		
		// Draw a point at the look location.
		DrawDebugPoint(GetWorld(), ActorToLookAt->GetActorLocation(), 10.f, FColor::Red, false, -1.f, 0.f);
		
		NormalizedLookDirection = GetActorTransform().InverseTransformVector(NormalizedLookDirection);
		// Normalize the look direction
		//NormalizedLookDirection = NormalizedLookDirection.GetSafeNormal();
	} else {
		LookAtTarget = false;
	}
}


void ATP_ThirdPersonCharacter::SetAiming(bool bNewIsAiming)
{
	if (bNewIsAiming)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("LookMode: Aiming"));

		LookMode = LookMode_Aiming;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("LookMode: Normal"));

		LookMode = LookMode_None;
	}

	bIsAiming = bNewIsAiming;
}

void ATP_ThirdPersonCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ATP_ThirdPersonCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ATP_ThirdPersonCharacter::UpdateMovementSpeed()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Test"));
	
	if (IsSprinting)
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
	else
		GetCharacterMovement()->MaxWalkSpeed = 200.f;
}

void ATP_ThirdPersonCharacter::GetCharacterInfo_Implementation(FVector& nLD, TEnumAsByte<ELookMode>& lM, bool& hLT)
{
	ICharacterDataInterface::GetCharacterInfo_Implementation(nLD, lM, hLT);
	
	nLD = this->NormalizedLookDirection;
	lM = this->LookMode;
	hLT = this->LookAtTarget;
}


void ATP_ThirdPersonCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		
		AddMovementInput(Direction, Value);
	}
}

void ATP_ThirdPersonCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
