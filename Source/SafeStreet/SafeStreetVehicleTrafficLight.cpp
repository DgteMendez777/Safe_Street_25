// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetVehicleTrafficLight.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ASafeStreetVehicleTrafficLight::ASafeStreetVehicleTrafficLight()
{
	PrimaryActorTick.bCanEverTick = false;

	TrafficLightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrafficLightMesh"));
	SetRootComponent(TrafficLightMesh);
}

// Called when the game starts or when spawned
void ASafeStreetVehicleTrafficLight::BeginPlay()
{
	Super::BeginPlay();

	if (TrafficLightMesh)
	{
		RedMID = TrafficLightMesh->CreateDynamicMaterialInstance(RedMaterialSlot);
		YellowMID = TrafficLightMesh->CreateDynamicMaterialInstance(YellowMaterialSlot);
		GreenMID = TrafficLightMesh->CreateDynamicMaterialInstance(GreenMaterialSlot);
	}

	// Followers always start red and wait for their leader; BeginPlay order between actors
	// isn't guaranteed, so a follower must never start itself from InitialState.
	SetLightState(bIsPhaseLeader ? InitialState : EVehicleTrafficLightState::Red);
}

void ASafeStreetVehicleTrafficLight::SetLightState(EVehicleTrafficLightState NewState)
{
	CurrentState = NewState;

	ApplyBulbMaterials(NewState);
	OnStateChanged(NewState);
	NotifyPairedLight(NewState);

	// Followers wait at red until their leader tells them to go, instead of timing out on their own
	const bool bFollowerWaitingAtRed = !bIsPhaseLeader && NewState == EVehicleTrafficLightState::Red;
	if (bFollowerWaitingAtRed)
	{
		GetWorldTimerManager().ClearTimer(StateTimerHandle);
		return;
	}

	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &ASafeStreetVehicleTrafficLight::AdvanceState, GetDurationForState(NewState), false);
}

void ASafeStreetVehicleTrafficLight::NotifyPairedLight(EVehicleTrafficLightState NewState) const
{
	// Only the leader drives the pair, so the follower's own state changes don't bounce back and reset it
	if (!bIsPhaseLeader || !PairedTrafficLight)
	{
		return;
	}

	switch (NewState)
	{
	case EVehicleTrafficLightState::Green:
	case EVehicleTrafficLightState::Yellow:
		PairedTrafficLight->SetLightState(EVehicleTrafficLightState::Red);
		break;
	case EVehicleTrafficLightState::Red:
		PairedTrafficLight->SetLightState(EVehicleTrafficLightState::Green);
		break;
	}
}

void ASafeStreetVehicleTrafficLight::ApplyBulbMaterials(EVehicleTrafficLightState NewState)
{
	if (RedMID)
	{
		RedMID->SetScalarParameterValue(EmissiveIntensityParamName, NewState == EVehicleTrafficLightState::Red ? 1.f : 0.f);
	}
	if (YellowMID)
	{
		YellowMID->SetScalarParameterValue(EmissiveIntensityParamName, NewState == EVehicleTrafficLightState::Yellow ? 1.f : 0.f);
	}
	if (GreenMID)
	{
		GreenMID->SetScalarParameterValue(EmissiveIntensityParamName, NewState == EVehicleTrafficLightState::Green ? 1.f : 0.f);
	}
}

float ASafeStreetVehicleTrafficLight::GetDurationForState(EVehicleTrafficLightState State) const
{
	switch (State)
	{
	case EVehicleTrafficLightState::Green:
		return GreenDuration;
	case EVehicleTrafficLightState::Yellow:
		return YellowDuration;
	case EVehicleTrafficLightState::Red:
		return RedDuration;
	default:
		return 1.f;
	}
}

void ASafeStreetVehicleTrafficLight::AdvanceState()
{
	EVehicleTrafficLightState NextState = CurrentState;
	switch (CurrentState)
	{
	case EVehicleTrafficLightState::Green:
		NextState = EVehicleTrafficLightState::Yellow;
		break;
	case EVehicleTrafficLightState::Yellow:
		NextState = EVehicleTrafficLightState::Red;
		break;
	case EVehicleTrafficLightState::Red:
		NextState = EVehicleTrafficLightState::Green;
		break;
	}

	SetLightState(NextState);
}
