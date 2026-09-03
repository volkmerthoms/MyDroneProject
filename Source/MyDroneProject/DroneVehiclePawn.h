// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DroneVehiclePawn.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

UENUM(BlueprintType)
enum class EDroneFlightState : uint8
{
	Idle,
	Flying,
	Crashed,
	BatteryDepleted
};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model")
	float CrashImpactSpeedThreshold = 700.f;

	// --- Battery ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Battery")
	float BatteryDrainPerSecondAtFullThrottle = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Battery")
	float BatteryIdleDrainPerSecond = 0.15f;

	UPROPERTY(BlueprintReadOnly, Category = "Drone|Battery")
	float BatteryPercent = 100.f;

	// --- Runtime state readable from Blueprint / HUD ---
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Drone|State")
	EDroneFlightState FlightState = EDroneFlightState::Idle;

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

	UFUNCTION(BlueprintCallable, Category = "Drone")
	void ResetDrone();

	UFUNCTION(BlueprintCallable, Category = "Drone")
	void SetMotorsArmed(bool bArmed);

	// --- Hooks for cosmetic feedback in Blueprint subclasses ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Drone|Events")
	void OnDroneCrashed(float ImpactSpeed);
	UFUNCTION(BlueprintImplementableEvent, Category = "Drone|Events")
	void OnBatteryDepleted();
	UFUNCTION(BlueprintImplementableEvent, Category = "Drone|Events")
	void OnMotorsArmedChanged(bool bArmed);

private:
	UFUNCTION()
	void HandleBodyHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void ApplyFlightForces(float DeltaSeconds);
	void UpdatePropellerVisuals(float DeltaSeconds);
	void UpdateBattery(float DeltaSeconds);

	bool bMotorsArmed = true;
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

	// TeleportPhysics in ResetDrone() can leave BodyMesh touching/penetrating level geometry at the
	// spawn point; the very next physics substep then resolves that overlap with a depenetration
	// impulse that HandleBodyHit would otherwise misread as a fresh crash. Suppress hit detection for
	// a short window after any reset so that push-out doesn't immediately re-trigger Crashed.
	float CrashDetectionGraceSeconds = 0.f;
};
