// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "DragonLocomotionCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class EDragonLocomotionState : uint8
{
    Grounded UMETA(DisplayName = "Grounded"),
    TakingOff UMETA(DisplayName = "Takeoff"),
    Flying UMETA(DisplayName = "Flying"),
    Gliding UMETA(DisplayName = "Gliding"),
    Diving UMETA(DisplayName = "Diving"),
    Landing UMETA(DisplayName = "Landing"),
};


UENUM(BlueprintType)
enum class EDragonJumpState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Moving UMETA(DisplayName = "Moving"),
    Charge UMETA(DisplayName = "Charge"),
};

/**

- A simple player-controllable third person character
- Implements a controllable orbiting camera
*/
UCLASS(abstract)
class ADragonLocomotionCharacter : public ACharacter
{
    GENERATED_BODY()

    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;


protected:

     
    /** Current dragon locomotion state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight")
    EDragonLocomotionState LocomotionState = EDragonLocomotionState::Grounded;

    /** Movement speed while walking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 250.0f;

    /** Movement speed while running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 500.0f;

    /** Movement speed while charging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float ChargeSpeed = 900.0f;

    /** True while the dragon is charging */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsCharging = false;

    /** Analog stick input magnitude required to transition from walking to running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkRunThreshold = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float FlightSpeed = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Flight")
    float MinimumFlightSpeed = 300.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight")
    float FlightYawInput = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight")
    float FlightPitchInput = 0.0f;

    /** Calculate target banking angle */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight")
    float TargetRoll = 0.f;

    /** Calcluate target pitch */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight")
    float TargetPitch = 0.0f;

    /** Camera yaw offset while flying, relative to the dragon */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight/Camera")
    float FlightCameraYawOffset = 0.0f;

    /** Maximum camera yaw offset while flying */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight/Camera")
    float FlightCameraYawLimit = 45.0f;

    /** Camera pitch offset while flying, relative to the dragon */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight/Camera")
    float FlightCameraPitchOffset = 0.0f;

    /** Maximum camera pitch offset while flying */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight/Camera")
    float FlightCameraPitchLimit = 20.0f;

    /** Rate of change during Dragon's fall */
    UPROPERTY(EditAnywhere, Category = "Gliding")
    float GlideFallRate = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Flying")
    float FlightGracePeriod = 3.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flying")
    float TimeSinceLastFlap = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Diving")
    float DiveAcceleration = 1200.0f;

    UPROPERTY(EditAnywhere, Category = "Diving")
    float MaxDiveSpeed = 2400.0f;

    UPROPERTY(EditAnywhere, Category = "Diving")
    float DivePitch = -50.0f;

    UPROPERTY(EditAnywhere, Category = "Diving")
    float DivePitchInterpSpeed = 4.0f;

    UPROPERTY(EditAnywhere, Category = "Gliding")
    float GlideDrag = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Flight")
    float FlightAcceleration = 4.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flying")
    bool bIsFlapping = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsJumping = false;


    /** Jump Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* JumpAction;

    /** Move Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveAction;

    /** Charge Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* ChargeAction;

    /** Look Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* LookAction;

    /** Mouse Look Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MouseLookAction;

    /** Flight Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* FlightAction;

    /** Dive Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* DiveAction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    EDragonJumpState JumpState = EDragonJumpState::Idle;
     

public:

    /** Constructor */
    ADragonLocomotionCharacter();

    /** Called every frame */
    virtual void Tick(float DeltaTime) override;
     

protected:

    /** Initialize input action bindings */
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
     

protected:

    /** Called for movement input */
    void Move(const FInputActionValue& Value);

    /** Called for looking input */
    void Look(const FInputActionValue& Value);

    void UpdateTakeoff(float DeltaTime);

    void UpdateFlight(float DeltaTime);

    void UpdateGlide(float DeltaTime);

    void UpdateLanding(float DeltaTime); 

    void EnterGroundedState();

    bool IsGroundDetected() const;

    void UpdateDive(float DeltaTime);

public:

     
    /** Handles move inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoMove(float Right, float Forward);

    /** Handles look inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoLook(float Yaw, float Pitch);

    /** Handles jump pressed inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpStart();

    /** Handles jump pressed inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpEnd();

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void StartCharge();

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void StopCharge();

    /** Handles flight takeoff inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void OnFlightPressed();

    /** Handles flight takeoff inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void OnFlightReleased();

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void OnDivePressed(); 

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void OnDiveReleased();
     

public:

     
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
     

};