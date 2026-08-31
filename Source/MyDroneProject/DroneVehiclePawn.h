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
	float ArmLengthCm = 45.f;

	UFUNCTION(BlueprintCallable, Category = "Drone")
	void ToggleCameraView();

	UFUNCTION(BlueprintCallable, Category = "Drone|Camera")
	void AddCameraLookInput(float YawDelta, float PitchDelta);

private:
	bool bUsingFpvCamera = false;

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
};
