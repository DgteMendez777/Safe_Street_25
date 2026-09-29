// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetPedestrianNPC.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ASafeStreetPedestrianNPC::ASafeStreetPedestrianNPC()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 300.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
}

void ASafeStreetPedestrianNPC::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyAppearance();
}

// Called when the game starts or when spawned
void ASafeStreetPedestrianNPC::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	ApplyAppearance();

	CurrentCheckpointIndex = 0;
}

void ASafeStreetPedestrianNPC::ApplyAppearance()
{
	if (!GetMesh())
	{
		return;
	}

	if (CharacterMesh)
	{
		GetMesh()->SetSkeletalMesh(CharacterMesh);
	}

	if (WalkAnimation)
	{
		// No Animation Blueprint needed: this NPC only ever walks, so just loop the chosen clip directly.
		GetMesh()->PlayAnimation(WalkAnimation, true);
	}
}

// Called every frame
void ASafeStreetPedestrianNPC::Tick(float DeltaTime)
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

	const FVector ToTarget = TargetCheckpoint->GetActorLocation() - GetActorLocation();

	if (ToTarget.SizeSquared() <= FMath::Square(AcceptanceRadius))
	{
		CurrentCheckpointIndex++;

		if (!Checkpoints.IsValidIndex(CurrentCheckpointIndex))
		{
			CurrentCheckpointIndex = bLoop ? 0 : Checkpoints.Num() - 1;
		}
		return;
	}

	AddMovementInput(ToTarget.GetSafeNormal());
}
