// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DronePlayerController.generated.h"

class ADroneVehiclePawn;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Video 3: builds its Enhanced Input actions and mapping context entirely in C++ (no .uasset
 * data assets required), then forwards axis values to the possessed ADroneVehiclePawn.
 */
UCLASS()
class MYDRONEPROJECT_API ADronePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	void BuildInputAssets();

	void OnThrottle(const FInputActionValue& Value);
	void OnPitch(const FInputActionValue& Value);
	void OnRoll(const FInputActionValue& Value);
	void OnYaw(const FInputActionValue& Value);
	void OnLookYaw(const FInputActionValue& Value);
	void OnLookPitch(const FInputActionValue& Value);
	void OnToggleCamera(const FInputActionValue& Value);
	void OnResetDrone(const FInputActionValue& Value);

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> DroneMappingContext;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_Throttle;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_Pitch;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_Roll;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_Yaw;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_LookYaw;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_LookPitch;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_ToggleCamera;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_Reset;

	UPROPERTY(Transient)
	TObjectPtr<ADroneVehiclePawn> ControlledDrone;
};
