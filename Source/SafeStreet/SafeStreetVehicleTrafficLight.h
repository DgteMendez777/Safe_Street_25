// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SafeStreetVehicleTrafficLight.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EVehicleTrafficLightState : uint8
{
	Green,
	Yellow,
	Red
};

UCLASS()
class SAFESTREET_API ASafeStreetVehicleTrafficLight : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASafeStreetVehicleTrafficLight();

	// Forces the light into a given state and restarts the cycle timer from there
	UFUNCTION(BlueprintCallable, Category = "Traffic Light")
	void SetLightState(EVehicleTrafficLightState NewState);

	UFUNCTION(BlueprintPure, Category = "Traffic Light")
	EVehicleTrafficLightState GetLightState() const { return CurrentState; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Notifies Blueprints whenever the active bulb changes, for extras beyond the emissive swap (sound, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Traffic Light")
	void OnStateChanged(EVehicleTrafficLightState NewState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic Light", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> TrafficLightMesh;

	// Material slot index on TrafficLightMesh for each bulb
	UPROPERTY(EditAnywhere, Category = "Traffic Light|Material Slots")
	int32 RedMaterialSlot = 5;

	UPROPERTY(EditAnywhere, Category = "Traffic Light|Material Slots")
	int32 YellowMaterialSlot = 3;

	UPROPERTY(EditAnywhere, Category = "Traffic Light|Material Slots")
	int32 GreenMaterialSlot = 6;

	// Scalar parameter name each bulb material exposes to drive its emissive strength (0 = off, 1 = on)
	UPROPERTY(EditAnywhere, Category = "Traffic Light|Material Slots")
	FName EmissiveIntensityParamName = "EmissiveIntensity";

	UPROPERTY(EditAnywhere, Category = "Traffic Light|Timing", meta = (ClampMin = "0.0"))
	float GreenDuration = 20.f;

	UPROPERTY(EditAnywhere, Category = "Traffic Light|Timing", meta = (ClampMin = "0.0"))
	float YellowDuration = 2.f;

	UPROPERTY(EditAnywhere, Category = "Traffic Light|Timing", meta = (ClampMin = "0.0"))
	float RedDuration = 20.f;

	// State the cycle starts in when the level begins
	UPROPERTY(EditAnywhere, Category = "Traffic Light|Timing")
	EVehicleTrafficLightState InitialState = EVehicleTrafficLightState::Green;

	// The light on the crossing street, kept in the opposite phase. Only set this on the phase leader.
	UPROPERTY(EditInstanceOnly, Category = "Traffic Light|Sync")
	TObjectPtr<ASafeStreetVehicleTrafficLight> PairedTrafficLight;

	// Leaders run their own Green/Yellow/Red timer forever. Followers sit at Red and only change
	// when their paired leader tells them to, so the two can never both show green at once.
	UPROPERTY(EditAnywhere, Category = "Traffic Light|Sync")
	bool bIsPhaseLeader = true;

private:
	void ApplyBulbMaterials(EVehicleTrafficLightState NewState);
	void AdvanceState();
	void NotifyPairedLight(EVehicleTrafficLightState NewState) const;
	float GetDurationForState(EVehicleTrafficLightState State) const;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RedMID;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> YellowMID;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GreenMID;

	EVehicleTrafficLightState CurrentState = EVehicleTrafficLightState::Green;

	FTimerHandle StateTimerHandle;
};
