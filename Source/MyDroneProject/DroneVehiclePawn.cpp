// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneVehiclePawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"

ADroneVehiclePawn::ADroneVehiclePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	SetRootComponent(BodyMesh);
	BodyMesh->SetSimulatePhysics(true);
	BodyMesh->SetEnableGravity(true);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyMesh->SetCollisionObjectType(ECC_PhysicsBody);
	BodyMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BodyMesh->SetLinearDamping(0.6f);
	BodyMesh->SetAngularDamping(3.f);
	BodyMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.12f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (BodyMeshAsset.Succeeded())
	{
		BodyMesh->SetStaticMesh(BodyMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PropMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	// X-frame layout: front-right/front-left/rear-left/rear-right, matching typical quad naming.
	constexpr float RotorAnglesDeg[4] = { 45.f, 135.f, 225.f, 315.f };
	const TCHAR* RotorNames[4] = { TEXT("RotorFrontRight"), TEXT("RotorFrontLeft"), TEXT("RotorRearLeft"), TEXT("RotorRearRight") };

	for (int32 i = 0; i < 4; ++i)
	{
		USceneComponent* RotorPoint = CreateDefaultSubobject<USceneComponent>(*FString::Printf(TEXT("%s_Point"), RotorNames[i]));
		RotorPoint->SetupAttachment(BodyMesh);
		const float AngleRad = FMath::DegreesToRadians(RotorAnglesDeg[i]);
		RotorPoint->SetRelativeLocation(FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f) * ArmLengthCm);
		RotorPoints.Add(RotorPoint);

		UStaticMeshComponent* PropMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("%s_Mesh"), RotorNames[i]));
		PropMesh->SetupAttachment(RotorPoint);
		PropMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PropMesh->SetCastShadow(false);
		PropMesh->SetRelativeScale3D(FVector(0.45f, 0.15f, 0.02f));
		if (PropMeshAsset.Succeeded())
		{
			PropMesh->SetStaticMesh(PropMeshAsset.Object);
		}
		PropellerMeshes.Add(PropMesh);
	}

	ChaseCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseCameraBoom"));
	ChaseCameraBoom->SetupAttachment(BodyMesh);
	ChaseCameraBoom->TargetArmLength = 450.f;
	ChaseCameraBoom->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	ChaseCameraBoom->bUsePawnControlRotation = false;
	ChaseCameraBoom->bDoCollisionTest = false;
	ChaseCameraBoom->bEnableCameraLag = true;
	ChaseCameraBoom->CameraLagSpeed = 4.f;
	ChaseCameraBoom->bInheritPitch = false;
	ChaseCameraBoom->bInheritYaw = true;
	ChaseCameraBoom->bInheritRoll = false;

	ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	ChaseCamera->SetupAttachment(ChaseCameraBoom, USpringArmComponent::SocketName);
	ChaseCamera->SetActive(true);

	FpvCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FpvCamera"));
	FpvCamera->SetupAttachment(BodyMesh);
	FpvCamera->SetRelativeLocation(FVector(15.f, 0.f, 5.f));
	FpvCamera->SetActive(false);

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void ADroneVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	SpawnRotation = GetActorRotation();

	BodyMesh->SetMassOverrideInKg(NAME_None, MassKg, true);
	BodyMesh->OnComponentHit.AddDynamic(this, &ADroneVehiclePawn::HandleBodyHit);

	FlightState = EDroneFlightState::Flying;
	CrashDetectionGraceSeconds = 0.25f;
}

void ADroneVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CrashDetectionGraceSeconds > 0.f)
	{
		CrashDetectionGraceSeconds = FMath::Max(0.f, CrashDetectionGraceSeconds - DeltaSeconds);
	}

	ApplyFlightForces(DeltaSeconds);
	UpdateBattery(DeltaSeconds);
	UpdatePropellerVisuals(DeltaSeconds);
}

void ADroneVehiclePawn::ApplyFlightForces(float DeltaSeconds)
{
	if (!bMotorsArmed || FlightState == EDroneFlightState::Crashed)
	{
		return;
	}

	SmoothedPitch = FMath::FInterpTo(SmoothedPitch, RawPitchInput, DeltaSeconds, InputSmoothingSpeed);
	SmoothedRoll = FMath::FInterpTo(SmoothedRoll, RawRollInput, DeltaSeconds, InputSmoothingSpeed);
	SmoothedYaw = FMath::FInterpTo(SmoothedYaw, RawYawInput, DeltaSeconds, InputSmoothingSpeed);

	// Throttle is a rate command, not a level: holding Space climbs, holding LeftShift descends,
	// and releasing both holds the current throttle - like a real quad's non-centering throttle
	// stick. This lets the drone actually be trimmed to a hover instead of free-falling the
	// instant the climb key is released.
	CurrentThrottle01 = FMath::Clamp(CurrentThrottle01 + RawThrottleInput * ThrottleRatePerSecond * DeltaSeconds, 0.f, 1.f);

	// Lift is applied along BodyMesh's own up axis (not world up) so that banking the drone also
	// tilts the thrust vector, giving it horizontal thrust the same way a real quad translates by
	// tilting into the direction of travel.
	const float TotalLiftForce = CurrentThrottle01 * MaxThrustPerRotor * FMath::Max(RotorPoints.Num(), 1);
	BodyMesh->AddForce(BodyMesh->GetUpVector() * TotalLiftForce);

	// Attitude self-level: axis-angle PD controller, not an Euler-angle one. It rotates BodyMesh's
	// up vector toward a target up vector tilted by the smoothed pitch/roll input (at the drone's
	// current heading), using FQuat::FindBetweenNormals to get the correction axis.
	const FRotator CurrentRotation = GetActorRotation();
	const FVector CurrentUp = BodyMesh->GetUpVector();
	const FVector AngularVelocityDeg = BodyMesh->GetPhysicsAngularVelocityInDegrees();

	// Flying forward (W, positive pitch input) needs the nose to dip down, i.e. negative UE pitch -
	// hence the sign flip here, verified against PIE where the unflipped version flew backwards.
	const FRotator TargetTiltRotation(-SmoothedPitch * MaxTiltAngleDegrees, CurrentRotation.Yaw, SmoothedRoll * MaxTiltAngleDegrees);
	const FVector TargetUp = TargetTiltRotation.Quaternion().GetUpVector();

	const FQuat TiltCorrection = FQuat::FindBetweenNormals(CurrentUp, TargetUp);
	const FVector TiltAxis = TiltCorrection.GetRotationAxis();
	const float TiltErrorDeg = FMath::RadiansToDegrees(TiltCorrection.GetAngle());
	const float TiltDampingTorque = FVector::DotProduct(AngularVelocityDeg, TiltAxis) * AttitudeDampingGain;
	const FVector TiltTorque = TiltAxis * (TiltErrorDeg * AttitudeProportionalGain - TiltDampingTorque);

	// Yaw is a separate rate command around the drone's own (possibly tilted) up axis.
	const float YawDampingTorque = FVector::DotProduct(AngularVelocityDeg, CurrentUp) * AttitudeDampingGain;
	const FVector YawTorque = CurrentUp * (SmoothedYaw * YawRateDegreesPerSecond * YawProportionalGain - YawDampingTorque);

	BodyMesh->AddTorqueInDegrees(TiltTorque + YawTorque, NAME_None, true);
}

void ADroneVehiclePawn::UpdatePropellerVisuals(float DeltaSeconds)
{
	const float SpinSpeedDegPerSec = 180.f + CurrentThrottle01 * 2400.f;
	PropellerSpinDegrees = FMath::Fmod(PropellerSpinDegrees + SpinSpeedDegPerSec * DeltaSeconds, 360.f);

	for (int32 i = 0; i < PropellerMeshes.Num(); ++i)
	{
		if (PropellerMeshes[i])
		{
			const float Direction = (i % 2 == 0) ? 1.f : -1.f;
			PropellerMeshes[i]->SetRelativeRotation(FRotator(0.f, PropellerSpinDegrees * Direction, 0.f));
		}
	}
}

void ADroneVehiclePawn::UpdateBattery(float DeltaSeconds)
{
	if (FlightState == EDroneFlightState::Crashed || BatteryPercent <= 0.f)
	{
		return;
	}

	const float Drain = BatteryIdleDrainPerSecond + CurrentThrottle01 * BatteryDrainPerSecondAtFullThrottle;
	BatteryPercent = FMath::Max(0.f, BatteryPercent - Drain * DeltaSeconds);

	if (BatteryPercent <= 0.f && FlightState != EDroneFlightState::BatteryDepleted)
	{
		FlightState = EDroneFlightState::BatteryDepleted;
		SetMotorsArmed(false);
		OnBatteryDepleted();
	}
}

void ADroneVehiclePawn::HandleBodyHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (FlightState == EDroneFlightState::Crashed || CrashDetectionGraceSeconds > 0.f)
	{
		return;
	}

	// Impulse = mass * delta-velocity, so this recovers an approximate impact speed without
	// needing to cache pre-collision velocity separately.
	const float ImpactSpeed = NormalImpulse.Size() / FMath::Max(MassKg, 0.01f);
	if (ImpactSpeed > CrashImpactSpeedThreshold)
	{
		FlightState = EDroneFlightState::Crashed;
		SetMotorsArmed(false);
		OnDroneCrashed(ImpactSpeed);
	}
}

void ADroneVehiclePawn::SetThrottleInput(float Value)
{
	RawThrottleInput = FMath::Clamp(Value, -1.f, 1.f);
}

void ADroneVehiclePawn::SetPitchInput(float Value)
{
	RawPitchInput = FMath::Clamp(Value, -1.f, 1.f);
}

void ADroneVehiclePawn::SetRollInput(float Value)
{
	RawRollInput = FMath::Clamp(Value, -1.f, 1.f);
}

void ADroneVehiclePawn::SetYawInput(float Value)
{
	RawYawInput = FMath::Clamp(Value, -1.f, 1.f);
}

void ADroneVehiclePawn::ToggleCameraView()
{
	bUsingFpvCamera = !bUsingFpvCamera;
	ChaseCamera->SetActive(!bUsingFpvCamera);
	FpvCamera->SetActive(bUsingFpvCamera);
}

void ADroneVehiclePawn::AddCameraLookInput(float YawDelta, float PitchDelta)
{
	FRotator BoomRotation = ChaseCameraBoom->GetRelativeRotation();
	BoomRotation.Yaw += YawDelta;
	BoomRotation.Pitch = FMath::Clamp(BoomRotation.Pitch + PitchDelta, -85.f, 20.f);
	ChaseCameraBoom->SetRelativeRotation(BoomRotation);
}

void ADroneVehiclePawn::ResetDrone()
{
	BodyMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	BodyMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	SetActorLocationAndRotation(SpawnLocation, SpawnRotation, false, nullptr, ETeleportType::TeleportPhysics);

	BatteryPercent = 100.f;
	FlightState = EDroneFlightState::Flying;
	SetMotorsArmed(true);
	CrashDetectionGraceSeconds = 0.25f;

	RawThrottleInput = RawPitchInput = RawRollInput = RawYawInput = 0.f;
	SmoothedPitch = SmoothedRoll = SmoothedYaw = 0.f;
	CurrentThrottle01 = 0.f;
}

void ADroneVehiclePawn::SetMotorsArmed(bool bArmed)
{
	bMotorsArmed = bArmed;
	if (bArmed && FlightState != EDroneFlightState::Crashed && FlightState != EDroneFlightState::BatteryDepleted)
	{
		FlightState = EDroneFlightState::Flying;
	}
	OnMotorsArmedChanged(bArmed);
}
