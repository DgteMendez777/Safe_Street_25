// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetCurbWarningZone.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "SafeStreetCharacter.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ASafeStreetCurbWarningZone::ASafeStreetCurbWarningZone()
{
	PrimaryActorTick.bCanEverTick = false;

	DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionBox"));
	SetRootComponent(DetectionBox);
	DetectionBox->SetBoxExtent(FVector(400.f, 200.f, 150.f));
	DetectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DetectionBox->SetGenerateOverlapEvents(true);

	BarrierMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrierMesh"));
	BarrierMesh->SetupAttachment(DetectionBox);
	BarrierMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BarrierMesh->SetCastShadow(false);
	// The cube is 100 cm per side and centered on its pivot, so lifting it by half its scaled
	// height makes the wall stand on the ground at the actor's origin.
	BarrierMesh->SetRelativeScale3D(FVector(8.f, 0.05f, 1.5f));
	BarrierMesh->SetRelativeLocation(FVector(0.f, 0.f, 75.f));
	BarrierMesh->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube")
	);

	if (CubeFinder.Succeeded())
	{
		BarrierMesh->SetStaticMesh(CubeFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HologramMaterialFinder(
		TEXT("/Game/Sci_Fi_Light/Static_Meshes/SM_Sci_Fi_Battery_Unit/Materials/Light.Light")
	);

	if (HologramMaterialFinder.Succeeded())
	{
		BarrierMesh->SetMaterial(0, HologramMaterialFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AppearSoundFinder(
		TEXT("/Game/WidgetsFedd/Resources/Sound/barrierappear.barrierappear")
	);

	if (AppearSoundFinder.Succeeded())
	{
		AppearSound = AppearSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> VoiceSoundFinder(
		TEXT("/Game/WidgetsFedd/Resources/Sound/Calzada.Calzada")
	);

	if (VoiceSoundFinder.Succeeded())
	{
		VoiceSound = VoiceSoundFinder.Object;
	}

	DetectionBox->OnComponentBeginOverlap.AddDynamic(this, &ASafeStreetCurbWarningZone::OnDetectionBeginOverlap);
	DetectionBox->OnComponentEndOverlap.AddDynamic(this, &ASafeStreetCurbWarningZone::OnDetectionEndOverlap);
}

void ASafeStreetCurbWarningZone::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<ASafeStreetCharacter>())
	{
		BarrierMesh->SetHiddenInGame(false);

		if (AppearSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, AppearSound, BarrierMesh->GetComponentLocation());
		}

		GetWorldTimerManager().SetTimer(VoiceTimerHandle, this, &ASafeStreetCurbWarningZone::PlayVoice, FMath::Max(VoiceDelay, 0.01f), false);
	}
}

void ASafeStreetCurbWarningZone::PlayVoice()
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (!VoiceSound || Now - LastVoiceTime < VoiceCooldown)
	{
		return;
	}

	LastVoiceTime = Now;
	UGameplayStatics::PlaySoundAtLocation(this, VoiceSound, BarrierMesh->GetComponentLocation());
}

void ASafeStreetCurbWarningZone::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA<ASafeStreetCharacter>())
	{
		BarrierMesh->SetHiddenInGame(true);
		GetWorldTimerManager().ClearTimer(VoiceTimerHandle);
	}
}
