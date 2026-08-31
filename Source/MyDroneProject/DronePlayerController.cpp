// Copyright Epic Games, Inc. All Rights Reserved.

#include "DronePlayerController.h"
#include "DroneVehiclePawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

void ADronePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledDrone = Cast<ADroneVehiclePawn>(InPawn);
}

void ADronePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	BuildInputAssets();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DroneMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(IA_Throttle, ETriggerEvent::Triggered, this, &ADronePlayerController::OnThrottle);
		EIC->BindAction(IA_Throttle, ETriggerEvent::Completed, this, &ADronePlayerController::OnThrottle);
		EIC->BindAction(IA_Pitch, ETriggerEvent::Triggered, this, &ADronePlayerController::OnPitch);
		EIC->BindAction(IA_Pitch, ETriggerEvent::Completed, this, &ADronePlayerController::OnPitch);
		EIC->BindAction(IA_Roll, ETriggerEvent::Triggered, this, &ADronePlayerController::OnRoll);
		EIC->BindAction(IA_Roll, ETriggerEvent::Completed, this, &ADronePlayerController::OnRoll);
		EIC->BindAction(IA_Yaw, ETriggerEvent::Triggered, this, &ADronePlayerController::OnYaw);
		EIC->BindAction(IA_Yaw, ETriggerEvent::Completed, this, &ADronePlayerController::OnYaw);
		EIC->BindAction(IA_LookYaw, ETriggerEvent::Triggered, this, &ADronePlayerController::OnLookYaw);
		EIC->BindAction(IA_LookPitch, ETriggerEvent::Triggered, this, &ADronePlayerController::OnLookPitch);
		EIC->BindAction(IA_ToggleCamera, ETriggerEvent::Started, this, &ADronePlayerController::OnToggleCamera);
	}
}

void ADronePlayerController::BuildInputAssets()
{
	if (DroneMappingContext)
	{
		return;
	}

	DroneMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Drone"));

	auto MakeAction = [this](const TCHAR* Name, EInputActionValueType ValueType)
	{
		UInputAction* Action = NewObject<UInputAction>(this, Name);
		Action->ValueType = ValueType;
		return Action;
	};

	IA_Throttle = MakeAction(TEXT("IA_Throttle"), EInputActionValueType::Axis1D);
	IA_Pitch = MakeAction(TEXT("IA_Pitch"), EInputActionValueType::Axis1D);
	IA_Roll = MakeAction(TEXT("IA_Roll"), EInputActionValueType::Axis1D);
	IA_Yaw = MakeAction(TEXT("IA_Yaw"), EInputActionValueType::Axis1D);
	IA_LookYaw = MakeAction(TEXT("IA_LookYaw"), EInputActionValueType::Axis1D);
	IA_LookPitch = MakeAction(TEXT("IA_LookPitch"), EInputActionValueType::Axis1D);
	IA_ToggleCamera = MakeAction(TEXT("IA_ToggleCamera"), EInputActionValueType::Boolean);

	// Throttle: Space climbs, LeftShift reduces toward/through zero (no reverse-thrust rotors,
	// so "descend" is really just "climb less than hover" - matches a real quad).
	DroneMappingContext->MapKey(IA_Throttle, EKeys::SpaceBar);
	DroneMappingContext->MapKey(IA_Throttle, EKeys::LeftShift).Modifiers.Add(NewObject<UInputModifierNegate>(this));

	DroneMappingContext->MapKey(IA_Pitch, EKeys::W);
	DroneMappingContext->MapKey(IA_Pitch, EKeys::S).Modifiers.Add(NewObject<UInputModifierNegate>(this));

	DroneMappingContext->MapKey(IA_Roll, EKeys::D);
	DroneMappingContext->MapKey(IA_Roll, EKeys::A).Modifiers.Add(NewObject<UInputModifierNegate>(this));

	DroneMappingContext->MapKey(IA_Yaw, EKeys::E);
	DroneMappingContext->MapKey(IA_Yaw, EKeys::Q).Modifiers.Add(NewObject<UInputModifierNegate>(this));

	DroneMappingContext->MapKey(IA_LookYaw, EKeys::MouseX);
	DroneMappingContext->MapKey(IA_LookPitch, EKeys::MouseY);

	DroneMappingContext->MapKey(IA_ToggleCamera, EKeys::C);
}

void ADronePlayerController::OnThrottle(const FInputActionValue& Value)
{
	if (ControlledDrone)
	{
		ControlledDrone->SetThrottleInput(Value.Get<float>());
	}
}

void ADronePlayerController::OnPitch(const FInputActionValue& Value)
{
	if (ControlledDrone)
	{
		ControlledDrone->SetPitchInput(Value.Get<float>());
	}
}

void ADronePlayerController::OnRoll(const FInputActionValue& Value)
{
	if (ControlledDrone)
	{
		ControlledDrone->SetRollInput(Value.Get<float>());
	}
}

void ADronePlayerController::OnYaw(const FInputActionValue& Value)
{
	if (ControlledDrone)
	{
		ControlledDrone->SetYawInput(Value.Get<float>());
	}
}

void ADronePlayerController::OnLookYaw(const FInputActionValue& Value)
{
	if (ControlledDrone)
	{
		ControlledDrone->AddCameraLookInput(Value.Get<float>(), 0.f);
	}
}

void ADronePlayerController::OnLookPitch(const FInputActionValue& Value)
{
	if (ControlledDrone)
	{
		ControlledDrone->AddCameraLookInput(0.f, Value.Get<float>());
	}
}

void ADronePlayerController::OnToggleCamera(const FInputActionValue& Value)
{
	if (ControlledDrone)
	{
		ControlledDrone->ToggleCameraView();
	}
}
