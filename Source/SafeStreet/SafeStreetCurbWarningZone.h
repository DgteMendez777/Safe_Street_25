// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SafeStreetCurbWarningZone.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USoundBase;

UCLASS()
class SAFESTREET_API ASafeStreetCurbWarningZone : public AActor
{
	GENERATED_BODY()

public:
	ASafeStreetCurbWarningZone();

protected:
	// Area that makes the barrier appear when the player enters it. Resize it in the level
	// to be wider than the barrier so the warning shows up before the player reaches it.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warning Zone")
	TObjectPtr<UBoxComponent> DetectionBox;

	// The barrier itself: a plain mesh whose only visible part is its material. Resize/move it
	// (relative to the actor) to line it up with the curb. Hidden in game until the player is close.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warning Zone")
	TObjectPtr<UStaticMeshComponent> BarrierMesh;

	// Played at the barrier each time it appears; leave empty for silent
	UPROPERTY(EditAnywhere, Category = "Warning Zone")
	TObjectPtr<USoundBase> AppearSound;

	// Voice warning played right after the barrier appears; leave empty for silent
	UPROPERTY(EditAnywhere, Category = "Warning Zone")
	TObjectPtr<USoundBase> VoiceSound;

	// Seconds between the barrier appearing and the voice starting, so they don't overlap
	UPROPERTY(EditAnywhere, Category = "Warning Zone", meta = (ClampMin = "0.0"))
	float VoiceDelay = 0.6f;

	// Minimum seconds between two voice warnings, so entering and leaving the zone doesn't spam it
	UPROPERTY(EditAnywhere, Category = "Warning Zone", meta = (ClampMin = "0.0"))
	float VoiceCooldown = 6.f;

private:
	void PlayVoice();

	FTimerHandle VoiceTimerHandle;
	float LastVoiceTime = -1000.f;

	UFUNCTION()
	void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
