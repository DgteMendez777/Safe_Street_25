// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SafeStreetCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class USafeStreetPauseWidget;
class USafeStreetGameOverWidget;
class USoundBase;

UCLASS()
class SAFESTREET_API ASafeStreetCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASafeStreetCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Spring arm that keeps the camera at a distance behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	// Third person follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// Enhanced Input mapping context, assigned on the Blueprint child
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	// Hold to run instead of walk
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	// Multiplier applied to mouse look input; lower this if camera rotation feels too fast
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ClampMax = "2.0"))
	float LookSensitivity = 0.3f;

	// Movement speed while walking (default)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 114.f;

	// Movement speed while holding the Sprint action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float RunSpeed = 380.f;

	// Footstep sound, played repeatedly while the character is moving
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Footsteps", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FootstepSound;

	// Seconds between footstep sounds while walking
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Footsteps", meta = (AllowPrivateAccess = "true", ClampMin = "0.05"))
	float WalkStepInterval = 0.5f;

	// Seconds between footstep sounds while sprinting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Footsteps", meta = (AllowPrivateAccess = "true", ClampMin = "0.05"))
	float RunStepInterval = 0.3f;

	// Background ambience, looped for the whole session. The sound asset needs "Looping" enabled.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambience", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> AmbienceSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambience", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float AmbienceVolume = 0.5f;

	// Montage played when a traffic vehicle hits the player; recovery happens automatically when it finishes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> CrashMontage;

	// Fired when a traffic vehicle hits the player, for extras beyond the animation (sound, camera shake, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle Collision")
	void OnHitByVehicle();

	// Seconds after a crash before Game Over, cutting the recovery short
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Collision", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float GameOverDelay = 2.f;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<USafeStreetPauseWidget> PauseWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<USafeStreetGameOverWidget> GameOverWidgetClass;

	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void OnCrashMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void TriggerGameOver();

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);
	void TogglePause();
	void UpdateFootsteps(float DeltaTime);

	// True while the crash reaction plays; blocks movement input until RecoverFromCrash is called
	bool bIsIncapacitated = false;

	bool bIsSprinting = false;
	float FootstepTimer = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<USafeStreetPauseWidget> PauseWidget;

	UPROPERTY(Transient)
	TObjectPtr<USafeStreetGameOverWidget> GameOverWidget;

	FTimerHandle GameOverTimerHandle;
};
