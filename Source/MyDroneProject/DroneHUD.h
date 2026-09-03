// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DroneHUD.generated.h"

class ADroneVehiclePawn;

/**
 * Video 6: telemetry HUD drawn via the legacy AHUD Canvas API - no UMG widgets.
 */
UCLASS()
class MYDRONEPROJECT_API ADroneHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

private:
	void DrawBar(float X, float Y, float Width, float Height, float Fill01, FLinearColor FillColor);

	UPROPERTY(Transient)
	TObjectPtr<ADroneVehiclePawn> Drone;

	float TimeSinceLastRefresh = 0.f;
	static constexpr float RefreshInterval = 0.1f;

	// Cached at RefreshInterval - string formatting for several lines every frame is
	// comparatively expensive, refreshing at 10 Hz reads as instant to a human.
	FString StatusLine;
	FString AltitudeLine;
	FString SpeedLine;
	FString ThrottleLine;
	FString BatteryLine;
	FString AttitudeLine;
	float CachedThrottle01 = 0.f;
	float CachedBatteryPercent = 0.f;
	float CachedHoverThrottle01 = 0.f;
};
