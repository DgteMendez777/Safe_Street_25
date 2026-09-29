// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SafeStreetPedestrianNPC.generated.h"

class USkeletalMesh;
class UAnimSequence;

UCLASS()
class SAFESTREET_API ASafeStreetPedestrianNPC : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASafeStreetPedestrianNPC();

protected:
	// Applies CharacterMesh/WalkAnimation, including live in the editor when you change them
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Which body this NPC uses. Any of your NPC characters works here.
	UPROPERTY(EditAnywhere, Category = "Pedestrian|Appearance")
	TObjectPtr<USkeletalMesh> CharacterMesh;

	// Walk animation this NPC plays. Any Mixamo walk clip works regardless of which mesh is chosen.
	UPROPERTY(EditAnywhere, Category = "Pedestrian|Appearance")
	TObjectPtr<UAnimSequence> WalkAnimation;

	// Route this NPC walks, in order. Placed in the level (e.g. Target Point actors) and assigned per-instance.
	UPROPERTY(EditInstanceOnly, Category = "Pedestrian|Route")
	TArray<TObjectPtr<AActor>> Checkpoints;

	// Walking speed in cm/s
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pedestrian", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float WalkSpeed = 140.f;

	// How close (cm) the NPC needs to get to a checkpoint before switching to the next one
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pedestrian", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float AcceptanceRadius = 80.f;

	// Loop back to the first checkpoint after reaching the last one
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pedestrian", meta = (AllowPrivateAccess = "true"))
	bool bLoop = true;

private:
	void ApplyAppearance();

	int32 CurrentCheckpointIndex = 0;
};
