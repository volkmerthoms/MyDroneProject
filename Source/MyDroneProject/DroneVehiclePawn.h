// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DroneVehiclePawn.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

/**
 * Video 2: component hierarchy only. No input, physics forces, or state machine yet - 
 * the drone will just sit there (or fall, once physics is simulated) until Video 4.
 */
UCLASS()
class MYDRONEPROJECT_API ADroneVehiclePawn : public APawn
{
	GENERATED_BODY()

public:
	ADroneVehiclePawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone|Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone|Components")
	TArray<TObjectPtr<USceneComponent>> RotorPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone|Components")
	TArray<TObjectPtr<UStaticMeshComponent>> PropellerMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone|Camera")
	TObjectPtr<USpringArmComponent> ChaseCameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone|Camera")
	TObjectPtr<UCameraComponent> ChaseCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone|Camera")
	TObjectPtr<UCameraComponent> FpvCamera;

	// --- Basic tuning (more flight-model properties are added in Video 4) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model")
	float MassKg = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model")
	float ArmLengthCm = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model", meta = (ClampMin = "0.1"))
	float MaxThrustPerRotor = 550.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model", meta = (ClampMin = "1.0", ClampMax = "60.0"))
	float MaxTiltAngleDegrees = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model")
	float YawRateDegreesPerSecond = 110.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model")
	float AttitudeProportionalGain = 18.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model")
	float AttitudeDampingGain = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model")
	float YawProportionalGain = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model")
	float InputSmoothingSpeed = 6.f;

	// Throttle is a rate command (climb/hold/descend), not a position - this is how fast it
	// ramps from 0 to 1 per second while the climb input is held.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model", meta = (ClampMin = "0.05"))
	float ThrottleRatePerSecond = 0.6f;

	UPROPERTY(BlueprintReadOnly, Category = "Drone|State")
	float CurrentThrottle01 = 0.f;

	// --- Input setters, called by the PlayerController ---
	UFUNCTION(BlueprintCallable, Category = "Drone|Input")
	void SetThrottleInput(float Value);
	UFUNCTION(BlueprintCallable, Category = "Drone|Input")
	void SetPitchInput(float Value);
	UFUNCTION(BlueprintCallable, Category = "Drone|Input")
	void SetRollInput(float Value);
	UFUNCTION(BlueprintCallable, Category = "Drone|Input")
	void SetYawInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Drone")
	void ToggleCameraView();

	UFUNCTION(BlueprintCallable, Category = "Drone|Camera")
	void AddCameraLookInput(float YawDelta, float PitchDelta);

private:
	void ApplyFlightForces(float DeltaSeconds);
	void UpdatePropellerVisuals(float DeltaSeconds);

	bool bUsingFpvCamera = false;

	float RawThrottleInput = 0.f;
	float RawPitchInput = 0.f;
	float RawRollInput = 0.f;
	float RawYawInput = 0.f;

	float SmoothedPitch = 0.f;
	float SmoothedRoll = 0.f;
	float SmoothedYaw = 0.f;

	float PropellerSpinDegrees = 0.f;

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
};
