// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DroneVehiclePawn.generated.h"

/**
 * Skeleton pawn for Video 1 - proves the custom C++ module and UCLASS/GENERATED_BODY
 * reflection setup compiles. Components, input, and flight physics are added in later videos.
 */
UCLASS()
class MYDRONEPROJECT_API ADroneVehiclePawn : public APawn
{
	GENERATED_BODY()

public:
	ADroneVehiclePawn();
};
