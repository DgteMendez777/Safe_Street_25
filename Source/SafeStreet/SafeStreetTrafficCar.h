// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SafeStreetTrafficCar.generated.h"

class UStaticMeshComponent;
class ASafeStreetVehicleTrafficLight;

// One intersection along the car's route: where it must stop, and which light governs it
USTRUCT(BlueprintType)
struct FTrafficLightCheckpoint
{
	GENERATED_BODY()

	// Actor placed at this intersection's stop line
	UPROPERTY(EditInstanceOnly, Category = "Vehicle|Traffic Light")
	TObjectPtr<AActor> StopPoint;

	// Traffic light the car checks once it's within range of StopPoint
	UPROPERTY(EditInstanceOnly, Category = "Vehicle|Traffic Light")
	TObjectPtr<ASafeStreetVehicleTrafficLight> TrafficLight;
};

UCLASS()
class SAFESTREET_API ASafeStreetTrafficCar : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASafeStreetTrafficCar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CarMesh;

	// Route this car follows, in order. Placed in the level (e.g. Target Point actors) and assigned per-instance.
	UPROPERTY(EditInstanceOnly, Category = "Vehicle|Route")
	TArray<TObjectPtr<AActor>> Checkpoints;

	// Movement speed in cm/s
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float Speed = 400.f;

	// How close (cm) the car needs to get to a checkpoint before switching to the next one
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float AcceptanceRadius = 150.f;

	// Degrees/sec used to smoothly turn the car towards its travel direction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float TurnRate = 90.f;

	// Loop back to the first checkpoint after reaching the last one
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	bool bLoop = true;

	// Every intersection this car passes through: its stop point and the light that governs it
	UPROPERTY(EditInstanceOnly, Category = "Vehicle|Traffic Light")
	TArray<FTrafficLightCheckpoint> TrafficLights;

	// Distance (cm) from a stop point at which the car halts if its paired light isn't green
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle|Traffic Light", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float TrafficLightStopDistance = 800.f;

	// Gap (cm) to keep from another traffic car ahead before stopping
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FollowDistance = 600.f;

private:
	bool ShouldStopForTrafficLight() const;
	bool IsCarAhead() const;

	int32 CurrentCheckpointIndex = 0;
};
