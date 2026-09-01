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
		PropMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.02f));
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
