// Copyright Epic Games, Inc. All Rights Reserved.

#include "DragonLocomotionCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "DragonLocomotion.h"

ADragonLocomotionCharacter::ADragonLocomotionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	 
		// Set size for collision capsule
		GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	 

}

void ADragonLocomotionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		 
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADragonLocomotionCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ADragonLocomotionCharacter::Move);

		// Camera
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ADragonLocomotionCharacter::Look);

		// Charging
		EnhancedInputComponent->BindAction(ChargeAction, ETriggerEvent::Started, this, &ADragonLocomotionCharacter::StartCharge);
		EnhancedInputComponent->BindAction(ChargeAction, ETriggerEvent::Completed, this, &ADragonLocomotionCharacter::StopCharge);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADragonLocomotionCharacter::Look);

		//Flight
		EnhancedInputComponent->BindAction(FlightAction, ETriggerEvent::Started, this, &ADragonLocomotionCharacter::OnFlightPressed);
		EnhancedInputComponent->BindAction(FlightAction, ETriggerEvent::Completed, this, &ADragonLocomotionCharacter::OnFlightReleased);
	}
	else
	{
		UE_LOG(LogDragonLocomotion, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
	 

}

void ADragonLocomotionCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	UE_LOG(
		LogDragonLocomotion,
		Warning,
		TEXT("MOVE INPUT - X: %f | Y: %f"),
		MovementVector.X,
		MovementVector.Y
	);

	DoMove(MovementVector.X, MovementVector.Y);
	 

}

void ADragonLocomotionCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	 
	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
	 
}

void ADragonLocomotionCharacter::UpdateTakeoff(float DeltaTime) {
	if (GetVelocity().Z < 0.f) {
		LocomotionState = EDragonLocomotionState::Flying;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);

		// Reset flight camera
		FlightCameraYawOffset = 0.0f;
		FlightCameraPitchOffset = 0.0f;

		// Center camera behind the dragon
		FRotator ControlRotation = GetControlRotation();
		ControlRotation.Yaw = GetActorRotation().Yaw;
		GetController()->SetControlRotation(ControlRotation);
	}
}

void ADragonLocomotionCharacter::UpdateFlight(float DeltaTime)
{
	if (FMath::Abs(FlightPitchInput) < 0.1f)
	{
		LocomotionState = EDragonLocomotionState::Gliding;
		return;
	}
	
	// Calculate target pitch and banking from input
	TargetPitch = FlightPitchInput * 45.0f;
	TargetRoll = FlightYawInput * 35.0f;

	// Get current rotation
	FRotator Rotation = GetActorRotation();

	// Apply pitch
	Rotation.Pitch = FMath::FInterpTo(
		Rotation.Pitch,
		TargetPitch,
		DeltaTime,
		3.0f);

	Rotation.Pitch = FMath::Clamp(
		Rotation.Pitch,
		-45.0f,
		45.0f
	);

	// Apply yaw
	Rotation.Yaw += FlightYawInput * 90.0f * DeltaTime;

	// Clamp pitch
	Rotation.Pitch = FMath::Clamp(Rotation.Pitch, -45.0f, 45.0f);

	// Smoothly bank into turns
	Rotation.Roll = FMath::FInterpTo(
		Rotation.Roll,
		TargetRoll,
		DeltaTime,
		5.0f
	);

	// Apply rotation
	SetActorRotation(Rotation);

	// Continuously move in the direction the dragon is facing
	AddMovementInput(GetActorForwardVector(), 1.0f);

	// Flight should use the dragon's orientation
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;

	// Keep camera behind the dragon with limited offset
	FRotator ControlRotation = GetControlRotation();
	ControlRotation.Yaw =
		GetActorRotation().Yaw + FlightCameraYawOffset;

	ControlRotation.Pitch =
		GetActorRotation().Pitch + FlightCameraPitchOffset;

	GetController()->SetControlRotation(ControlRotation);

}

void ADragonLocomotionCharacter::UpdateGlide(float DeltaTime) {
	// Continue moving forward
	AddMovementInput(GetActorForwardVector(), 1.0f);

	// Apply yaw
	FRotator Rotation = GetActorRotation();

	Rotation.Yaw += FlightYawInput * 90.0f * DeltaTime;

	// Apply banking
	TargetRoll = FlightYawInput * 35.0f;

	Rotation.Roll = FMath::FInterpTo(
		Rotation.Roll,
		TargetRoll,
		DeltaTime,
		5.0f
	);

	SetActorRotation(Rotation);

	// Gradually descend
	AddMovementInput(FVector::DownVector, GlideFallRate * DeltaTime);

	// Keep flight movement behavior
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;

	// Keep camera behind the dragon with limited offset
	FRotator ControlRotation = GetControlRotation();
	ControlRotation.Yaw =
		GetActorRotation().Yaw + FlightCameraYawOffset;

	ControlRotation.Pitch =
		GetActorRotation().Pitch + FlightCameraPitchOffset;

	GetController()->SetControlRotation(ControlRotation);

	UE_LOG(
		LogDragonLocomotion,
		Warning,
		TEXT("GLIDING - Z Velocity: %f"),
		GetCharacterMovement()->Velocity.Z
	);
}

void ADragonLocomotionCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FVector2D MovementInput(Right, Forward);

		 
		// Determine the magnitude of the input.
		const float InputMagnitude = MovementInput.Size();

		// Analog stick:
		// - Below threshold = Walk
		// - At or above threshold = Run
		//
		// Keyboard:
		// - WASD produces full-strength input, so it runs.

		if (LocomotionState == EDragonLocomotionState::Flying ||
			LocomotionState == EDragonLocomotionState::Gliding)
		{
			FlightYawInput = Right;
			FlightPitchInput = -Forward;

			return;
		}

		if (bIsCharging && Right == 0.0f && Forward == 0.0f) {
			Forward = 1.0f;
		}

		if (!bIsCharging) {
			if (InputMagnitude < WalkRunThreshold)
			{
				GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			}
			else
			{
				GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			}
		}

		// Find out which way is forward.
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		// Get forward vector.
		const FVector ForwardDirection =
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// Get right vector.
		const FVector RightDirection =
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
	 
	UE_LOG(
		LogDragonLocomotion,
		Warning,
		TEXT("Move Input - Right: %f | Forward: %f | State: %d"),
		Right,
		Forward,
		static_cast<int32>(LocomotionState)
	);

}

void ADragonLocomotionCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		if (LocomotionState == EDragonLocomotionState::Flying ||
			LocomotionState == EDragonLocomotionState::Gliding)
		{
			// Limit camera rotation relative to the dragon
			FlightCameraYawOffset = FMath::Clamp(
				FlightCameraYawOffset + Yaw,
				-FlightCameraYawLimit,
				FlightCameraYawLimit
			);

			// Limit vertical camera rotation relative to the dragon
			FlightCameraPitchOffset = FMath::Clamp(
				FlightCameraPitchOffset + Pitch,
				-FlightCameraPitchOffset,
				FlightCameraPitchOffset
			);

			// Allow normal vertical camera rotation
			AddControllerPitchInput(Pitch);

			// Keep camera yaw relative to dragon
			FRotator ControlRotation = GetControlRotation();
			
			ControlRotation.Yaw =
				GetActorRotation().Yaw + FlightCameraYawOffset;

			ControlRotation.Pitch =
				GetActorRotation().Pitch + FlightCameraPitchOffset;

			GetController()->SetControlRotation(ControlRotation);
		}
		else 
		{
			// Ground camera rotation is free
			AddControllerYawInput(Yaw);
			AddControllerPitchInput(Pitch);
		}
	}
}

void ADragonLocomotionCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ADragonLocomotionCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ADragonLocomotionCharacter::StartCharge()
{
	// signal the character to charge
	bIsCharging = true;

	GetCharacterMovement()->MaxWalkSpeed = ChargeSpeed;
	 

}

void ADragonLocomotionCharacter::StopCharge()
{
	// signal the character to stop charging
	bIsCharging = false;
}

void ADragonLocomotionCharacter::OnFlightPressed()
{
	UE_LOG(LogDragonLocomotion, Warning, TEXT("Flight Pressed"));

	 
		if (!GetCharacterMovement()->IsMovingOnGround())
		{
			return;
		}

	LocomotionState = EDragonLocomotionState::TakingOff;

	LaunchCharacter(FVector(0.0f, 0.0f, 700.0f), false, true);

	FlightSpeed = FMath::Clamp(
		GetVelocity().Length(),
		MinimumFlightSpeed,
		ChargeSpeed
	);

	GetCharacterMovement()->MaxFlySpeed = FlightSpeed;
	 

}

void ADragonLocomotionCharacter::OnFlightReleased()
{
	UE_LOG(LogDragonLocomotion, Warning, TEXT("Flight Released"));
}

void ADragonLocomotionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

		if (bIsCharging)
		{
			AddMovementInput(GetActorForwardVector(), 1.0f);
		}

	switch (LocomotionState)
	{
		case EDragonLocomotionState::TakingOff:
		
			// Tranistion into flight
			UpdateTakeoff(DeltaTime);
		
			break;

		case EDragonLocomotionState::Flying:
		
			// Continous flight
			UpdateFlight(DeltaTime);

			break;

		case EDragonLocomotionState::Gliding:
			
			// Transition into glide 
			UpdateGlide(DeltaTime);

			break;

		default:
			GetCharacterMovement()->bOrientRotationToMovement = true;
			bUseControllerRotationYaw = false;
			break;
		}

}