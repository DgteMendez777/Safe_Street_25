// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetTrafficCar.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SafeStreetVehicleTrafficLight.h"

// Sets default values
ASafeStreetTrafficCar::ASafeStreetTrafficCar()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarMesh"));
	SetRootComponent(CarMesh);
	CarMesh->SetMobility(EComponentMobility::Movable);
}

// Called when the game starts or when spawned
void ASafeStreetTrafficCar::BeginPlay()
{
	Super::BeginPlay();

	CurrentCheckpointIndex = 0;
}

// Called every frame
void ASafeStreetTrafficCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Checkpoints.IsValidIndex(CurrentCheckpointIndex))
	{
		return;
	}

	const AActor* TargetCheckpoint = Checkpoints[CurrentCheckpointIndex];
	if (!TargetCheckpoint)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = TargetCheckpoint->GetActorLocation();
	const FVector ToTarget = TargetLocation - CurrentLocation;

	if (ToTarget.SizeSquared() <= FMath::Square(AcceptanceRadius))
	{
		CurrentCheckpointIndex++;

		if (!Checkpoints.IsValidIndex(CurrentCheckpointIndex))
		{
			if (bLoop)
			{
				CurrentCheckpointIndex = 0;
			}
			else
			{
				CurrentCheckpointIndex = Checkpoints.Num() - 1;
				return;
			}
		}
		return;
	}

	if (ShouldStopForTrafficLight() || IsCarAhead())
	{
		return;
	}

	const FRotator DesiredRotation = ToTarget.GetSafeNormal().Rotation();
	const FRotator NewRotation = FMath::RInterpConstantTo(GetActorRotation(), DesiredRotation, DeltaTime, TurnRate);
	SetActorRotation(NewRotation);

	// Move along the car's own facing direction (post-turn) instead of straight at the target,
	// so the path curves through corners instead of the mesh sliding sideways while it rotates.
	SetActorLocation(CurrentLocation + NewRotation.Vector() * Speed * DeltaTime);
}

bool ASafeStreetTrafficCar::IsCarAhead() const
{
	TArray<AActor*> OtherCars;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASafeStreetTrafficCar::StaticClass(), OtherCars);

	const FVector CurrentLocation = GetActorLocation();
	const FVector Forward = GetActorForwardVector();

	for (AActor* OtherActor : OtherCars)
	{
		if (OtherActor == this)
		{
			continue;
		}

		const FVector ToOther = OtherActor->GetActorLocation() - CurrentLocation;
		if (ToOther.SizeSquared() > FMath::Square(FollowDistance))
		{
			continue;
		}

		// Only cars roughly ahead of us (within a ~90 degree forward cone) count, not ones beside or behind
		if (FVector::DotProduct(Forward, ToOther.GetSafeNormal()) > 0.f)
		{
			return true;
		}
	}

	return false;
}

bool ASafeStreetTrafficCar::ShouldStopForTrafficLight() const
{
	const FVector CurrentLocation = GetActorLocation();

	for (const FTrafficLightCheckpoint& Checkpoint : TrafficLights)
	{
		if (!Checkpoint.StopPoint || !Checkpoint.TrafficLight)
		{
			continue;
		}

		const float DistanceToStopPoint = FVector::Dist(CurrentLocation, Checkpoint.StopPoint->GetActorLocation());
		if (DistanceToStopPoint > TrafficLightStopDistance)
		{
			continue;
		}

		if (Checkpoint.TrafficLight->GetLightState() != EVehicleTrafficLightState::Green)
		{
			return true;
		}
	}

	return false;
}

