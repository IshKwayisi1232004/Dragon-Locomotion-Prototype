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
	GetCharacterMovement()->JumpZVelocity = 1000.f;
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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADragonLocomotionCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADragonLocomotionCharacter::StopJumping);

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

		// Diving 
		EnhancedInputComponent->BindAction(DiveAction, ETriggerEvent::Started, this, &ADragonLocomotionCharacter::OnDivePressed);

		EnhancedInputComponent->BindAction(DiveAction, ETriggerEvent::Completed, this, &ADragonLocomotionCharacter::OnDiveReleased);
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
	UE_LOG(
		LogDragonLocomotion,
		Warning,
		TEXT("GLIDE - OnGround: %d | MovementMode: %d | Velocity: %s"),
		GetCharacterMovement()->IsMovingOnGround(),
		GetCharacterMovement()->MovementMode,
		*GetVelocity().ToString()
	);

	if (GetCharacterMovement()->IsMovingOnGround())
	{
		EnterGroundedState();
		return;
	}

	TimeSinceLastFlap += DeltaTime;

	if (TimeSinceLastFlap >= FlightGracePeriod)
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

	// Preserve the Dragon's current momentum while flying
	FVector Velocity = GetVelocity();

	if (!Velocity.IsNearlyZero()) 
	{
		const float CurrentSpeed = Velocity.Size();

		// Gradually align the velocity with the Dragon's facing direction
		// while preserving its current speed
		const FVector TargetVelocity =
			GetActorForwardVector() * CurrentSpeed;

		Velocity = FMath::VInterpTo(
			Velocity,
			TargetVelocity,
			DeltaTime,
			FlightTurnSpeed
		);

		GetCharacterMovement()->Velocity = Velocity;
	}

	ApplyLift(DeltaTime);

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

void ADragonLocomotionCharacter::UpdateGlide(float DeltaTime)
{
	if (IsGroundDetected())
	{
		EnterGroundedState();
		return;
	}

	// Apply yaw.
	FRotator Rotation = GetActorRotation();

	Rotation.Yaw += FlightYawInput * 90.0f * DeltaTime;

	// Apply banking.
	TargetRoll = FlightYawInput * 35.0f;

	Rotation.Roll = FMath::FInterpTo(
		Rotation.Roll,
		TargetRoll,
		DeltaTime,
		5.0f
	);

	SetActorRotation(Rotation);

	// Preserve the Dragon's current momentum while gliding.
	FVector Velocity = GetVelocity();

	if (!Velocity.IsNearlyZero())
	{
		const float CurrentSpeed = Velocity.Size();

		// Gradually align the velocity with the Dragon's facing direction
		// while preserving its current speed.
		const FVector TargetVelocity =
			GetActorForwardVector() * CurrentSpeed;

		Velocity = FMath::VInterpTo(
			Velocity,
			TargetVelocity,
			DeltaTime,
			FlightTurnSpeed
		);

		// Apply gradual downward movement for gliding.
		Velocity += FVector::DownVector * GlideFallRate * DeltaTime;

		GetCharacterMovement()->Velocity = Velocity;
	}

	ApplyLift(DeltaTime);

	// Keep flight movement behavior.
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;

	// Keep camera behind the Dragon with limited offset.
	FRotator ControlRotation = GetControlRotation();

	ControlRotation.Yaw =
		GetActorRotation().Yaw + FlightCameraYawOffset;

	ControlRotation.Pitch =
		GetActorRotation().Pitch + FlightCameraPitchOffset;

	GetController()->SetControlRotation(ControlRotation);

	UE_LOG(
		LogDragonLocomotion,
		Warning,
		TEXT("GLIDING - Speed: %f | Z Velocity: %f"),
		Velocity.Size(),
		Velocity.Z
	);
}

void ADragonLocomotionCharacter::EnterGroundedState()
{
	LocomotionState = EDragonLocomotionState::Grounded;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->StopMovementImmediately();

	FlightYawInput = 0.0f;
	FlightPitchInput = 0.0f;
	TargetRoll = 0.0f;
	TargetPitch = 0.0f;

	TimeSinceLastFlap = 0.0f;

	UE_LOG(
		LogDragonLocomotion,
		Warning,
		TEXT("ENTERING GROUNDED - Velocity: %s | MovementMode: %d"),
		*GetVelocity().ToString(),
		GetCharacterMovement()->MovementMode
	);
}

bool ADragonLocomotionCharacter::IsGroundDetected() const {
	FHitResult Hit; 

	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.0f, 0.0f, 110.0f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	return GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		QueryParams
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
			LocomotionState == EDragonLocomotionState::Gliding || 
			LocomotionState == EDragonLocomotionState::Diving)
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
	FVector HorizontalVelocity(
		GetVelocity().X,
		GetVelocity().Y,
		0.0f
	);

	if (bIsCharging)
	{
		JumpState = EDragonJumpState::Charge;
	}
	else if (HorizontalVelocity.Size() > 10.0f)
	{
		JumpState = EDragonJumpState::Moving;
	}
	else
	{
		JumpState = EDragonJumpState::Idle;
	}

	bIsJumping = true;

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

	// Ground -> Takeoff
	if (LocomotionState == EDragonLocomotionState::Grounded)
	{
		LocomotionState = EDragonLocomotionState::TakingOff;

		// Preserve the Dragon's current horizontal momentum.
		const FVector CurrentVelocity = GetVelocity();
		const FVector CurrentHorizontalVelocity =
			FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);

		const float CurrentHorizontalSpeed = CurrentHorizontalVelocity.Size();

		// Guarantee a minimum forward speed when taking off.
		FlightSpeed = FMath::Clamp(
			FMath::Max(CurrentHorizontalSpeed, MinimumFlightSpeed),
			MinimumFlightSpeed,
			ChargeSpeed
		);

		// Use the Dragon's facing direction for the takeoff momentum.
		const FVector ForwardVelocity =
			GetActorForwardVector() * FlightSpeed;

		// Launch upward.
		LaunchCharacter(
			FVector(0.0f, 0.0f, 700.0f),
			false,
			true
		);

		// Combine forward momentum with the upward launch.
		GetCharacterMovement()->Velocity =
			ForwardVelocity + FVector(0.0f, 0.0f, 700.0f);

		GetCharacterMovement()->MaxFlySpeed = MaxDiveSpeed;

		TimeSinceLastFlap = 0.0f;

		return;
	}

	// Flying/Gliding -> Flap
	if (LocomotionState == EDragonLocomotionState::Flying ||
		LocomotionState == EDragonLocomotionState::Gliding)
	{
		LocomotionState = EDragonLocomotionState::Flying;
		TimeSinceLastFlap = 0.0f;

		return;
	}
}

void ADragonLocomotionCharacter::OnFlightReleased()
{
	UE_LOG(LogDragonLocomotion, Warning, TEXT("Flight Released"));
}

void ADragonLocomotionCharacter::UpdateDive(float DeltaTime)
{
	// Gradually pitch the Dragon downward
	TargetPitch = FMath::Clamp(
		DivePitch + (FlightPitchInput * 50.0f), 
		-80.0f,
		20.0f
	);

	const FRotator CurrentRotation = GetActorRotation();

	const float NewPitch = FMath::FInterpTo(
		CurrentRotation.Pitch,
		TargetPitch,
		DeltaTime,
		DivePitchInterpSpeed
	);

	SetActorRotation(FRotator(
		NewPitch,
		CurrentRotation.Yaw,
		CurrentRotation.Roll
	));

	// Accelerate in the direction the Dragon is facing
	FVector Velocity = GetVelocity();
	float CurrentSpeed = Velocity.Size();

	if (CurrentSpeed > 0.0f)
	{
		// Gradually redirect the velocity toward the direction
		// the Dragon is facing
		const FVector TargetVelocity =
			GetActorForwardVector() * CurrentSpeed;

		Velocity = FMath::VInterpTo(
			Velocity,
			TargetVelocity,
			DeltaTime,
			3.0f
		);
	}

	Velocity += GetActorForwardVector() * DiveAcceleration * DeltaTime;

	Velocity = Velocity.GetClampedToMaxSize(MaxDiveSpeed);

	GetCharacterMovement()->Velocity = Velocity; 

	// Keep the Dragon in flying movement mode while diving 
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

}

void ADragonLocomotionCharacter::OnDivePressed()
{
	// Only allow diving while airborne 
	if (LocomotionState != EDragonLocomotionState::Flying &&
		LocomotionState != EDragonLocomotionState::Gliding)
	{
		return; 
	}

	LocomotionState = EDragonLocomotionState::Diving; 

	// Make sure we're using flying movement
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	UE_LOG(
		LogDragonLocomotion,
		Warning,
		TEXT("ENTERING DIVE - Velocity: %s"),
		*GetVelocity().ToString()
	);
}

void ADragonLocomotionCharacter::OnDiveReleased()
{
	if (LocomotionState != EDragonLocomotionState::Diving)
	{
		return;
	}

	// Perserve the momentum gained during the dive
	const float CurrentSpeed = GetVelocity().Size();

	FlightSpeed = FMath::Max(CurrentSpeed, MinimumFlightSpeed);

	// Return to normal flight
	LocomotionState = EDragonLocomotionState::Flying;

	// Restore the normal flight pitch target
	TargetPitch = 0.0f;

	UE_LOG(
		LogDragonLocomotion,
		Warning,
		TEXT("Exiting DIVE - Flight Speed: %2f | Velocity: %s"),
		FlightSpeed,
		*GetVelocity().ToString()
	);
}

void ADragonLocomotionCharacter::SetLiftAcceleration(float NewLiftAcceleration)
{
	LiftAcceleration = NewLiftAcceleration;
}

float ADragonLocomotionCharacter::GetLiftAcceleration() const
{
	return LiftAcceleration;
}

void ADragonLocomotionCharacter::ApplyLift(float DeltaTime)
{
	if (FMath::IsNearlyZero(LiftAcceleration))
	{
		return;
	}

	FVector Velocity = GetCharacterMovement()->Velocity;

	Velocity.Z += LiftAcceleration * DeltaTime;

	GetCharacterMovement()->Velocity = Velocity; 
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
		case EDragonLocomotionState::Grounded:
			GetCharacterMovement()->bOrientRotationToMovement = true;
			bUseControllerRotationYaw = false;
			break;

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

		case EDragonLocomotionState::Diving:

			// Transition into dive
			UpdateDive(DeltaTime);

			break;

		default:
			GetCharacterMovement()->bOrientRotationToMovement = true;
			bUseControllerRotationYaw = false;
			break;
		}

}