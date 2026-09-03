// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneHUD.h"
#include "DroneVehiclePawn.h"
#include "Engine/Canvas.h"

void ADroneHUD::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetOwningPlayerController())
	{
		Drone = Cast<ADroneVehiclePawn>(PC->GetPawn());
	}
}

void ADroneHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	if (!Drone)
	{
		if (APlayerController* PC = GetOwningPlayerController())
		{
			Drone = Cast<ADroneVehiclePawn>(PC->GetPawn());
		}
	}

	ADroneVehiclePawn* DronePtr = Drone.Get();
	if (!DronePtr)
	{
		return;
	}

	TimeSinceLastRefresh += GetWorld()->GetDeltaSeconds();
	if (TimeSinceLastRefresh >= RefreshInterval)
	{
		TimeSinceLastRefresh = 0.f;

		const float Altitude = DronePtr->GetAltitudeAboveGround();
		const float SpeedKmh = DronePtr->GetSpeedKmh();
		const float ThrottlePct = DronePtr->CurrentThrottle01 * 100.f;
		const float HoverPct = DronePtr->GetHoverThrottleEstimate() * 100.f;
		const FRotator Rot = DronePtr->GetActorRotation();

		const TCHAR* StatusStr = TEXT("FLYING");
		switch (DronePtr->FlightState)
		{
			case EDroneFlightState::Idle:
				StatusStr = TEXT("IDLE");
				break;
			case EDroneFlightState::Crashed:
				StatusStr = TEXT("CRASHED - press R to reset");
				break;
			case EDroneFlightState::BatteryDepleted:
				StatusStr = TEXT("BATTERY DEPLETED - press R to reset");
				break;
			default:
				break;
		}

		StatusLine = StatusStr;
		AltitudeLine = FString::Printf(TEXT("Altitude: %.1f m"), Altitude);
		SpeedLine = FString::Printf(TEXT("Speed: %.1f km/h"), SpeedKmh);
		ThrottleLine = FString::Printf(TEXT("Throttle: %.0f%%  (Hover ~%.0f%%)"), ThrottlePct, HoverPct);
		BatteryLine = FString::Printf(TEXT("Battery: %.0f%%"), DronePtr->BatteryPercent);
		AttitudeLine = FString::Printf(TEXT("Pitch: %.1f deg  Roll: %.1f deg  Yaw: %.1f deg"), Rot.Pitch, Rot.Roll, Rot.Yaw);
		CachedThrottle01 = DronePtr->CurrentThrottle01;
		CachedBatteryPercent = DronePtr->BatteryPercent;
		CachedHoverThrottle01 = DronePtr->GetHoverThrottleEstimate();
	}

	const FLinearColor White = FLinearColor::White;
	constexpr float Left = 40.f;
	float Y = 40.f;
	constexpr float LineHeight = 26.f;

	DrawText(StatusLine, White, Left, Y, nullptr, 1.3f); Y += LineHeight;
	DrawText(AltitudeLine, White, Left, Y, nullptr, 1.3f); Y += LineHeight;
	DrawText(SpeedLine, White, Left, Y, nullptr, 1.3f); Y += LineHeight;
	DrawText(ThrottleLine, White, Left, Y, nullptr, 1.3f); Y += LineHeight;
	DrawText(BatteryLine, White, Left, Y, nullptr, 1.3f); Y += LineHeight * 1.4f;

	constexpr float BarWidth = 220.f;
	constexpr float BarHeight = 16.f;

	DrawBar(Left, Y, BarWidth, BarHeight, CachedThrottle01, FLinearColor(0.2f, 0.7f, 1.f));
	// Hover-throttle marker: a thin line over the throttle bar at the estimated hover point, so
	// trimming the throttle back to a steady hover after climbing is a visual target, not guesswork.
	const float HoverMarkerX = Left + BarWidth * FMath::Clamp(CachedHoverThrottle01, 0.f, 1.f);
	DrawRect(FLinearColor(1.f, 0.85f, 0.1f), HoverMarkerX - 1.f, Y, 2.f, BarHeight);
	Y += 24.f;

	DrawBar(Left, Y, BarWidth, BarHeight, CachedBatteryPercent / 100.f, FLinearColor(0.2f, 0.9f, 0.3f)); Y += LineHeight * 1.4f;

	DrawText(AttitudeLine, White, Left, Y, nullptr, 1.3f);
}

void ADroneHUD::DrawBar(float X, float Y, float Width, float Height, float Fill01, FLinearColor FillColor)
{
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), X, Y, Width, Height);
	DrawRect(FillColor, X, Y, Width * FMath::Clamp(Fill01, 0.f, 1.f), Height);
}
