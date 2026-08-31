// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneGameMode.h"
#include "DroneVehiclePawn.h"
#include "DronePlayerController.h"

ADroneGameMode::ADroneGameMode()
{
	DefaultPawnClass = ADroneVehiclePawn::StaticClass();
	PlayerControllerClass = ADronePlayerController::StaticClass();
}
