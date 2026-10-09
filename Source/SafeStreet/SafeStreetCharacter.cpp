// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetCharacter.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "SafeStreetGameOverWidget.h"
#include "SafeStreetPauseWidget.h"
#include "Sound/SoundBase.h"
#include "TrafficVehicle.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ASafeStreetCharacter::ASafeStreetCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// Only the controller's yaw rotates the camera boom; the mesh turns to face movement instead
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ASafeStreetCharacter::OnCapsuleBeginOverlap);

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultFootstepSoundFinder(
		TEXT("/Game/WidgetsFedd/Resources/Sound/step.step")
	);

	if (DefaultFootstepSoundFinder.Succeeded())
	{
		FootstepSound = DefaultFootstepSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultAmbienceSoundFinder(
		TEXT("/Game/WidgetsFedd/Resources/Sound/ambience.ambience")
	);

	if (DefaultAmbienceSoundFinder.Succeeded())
	{
		AmbienceSound = DefaultAmbienceSoundFinder.Object;
	}
}

// Called when the game starts or when spawned
void ASafeStreetCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		// The main menu leaves the controller in UI-only mode; make sure gameplay input works
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (AmbienceSound)
	{
		UGameplayStatics::PlaySound2D(this, AmbienceSound, AmbienceVolume);
	}
}

// Called every frame
void ASafeStreetCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateFootsteps(DeltaTime);
}

void ASafeStreetCharacter::UpdateFootsteps(float DeltaTime)
{
	if (bIsIncapacitated)
	{
		FootstepTimer = 0.f;
		return;
	}

	const bool bIsMoving = GetVelocity().SizeSquared() > FMath::Square(10.f);
	if (!bIsMoving)
	{
		FootstepTimer = 0.f;
		return;
	}

	const float Interval = bIsSprinting ? RunStepInterval : WalkStepInterval;

	FootstepTimer += DeltaTime;

	if (FootstepTimer >= Interval)
	{
		FootstepTimer = 0.f;

		if (FootstepSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FootstepSound, GetActorLocation());
		}
	}
}

// Called to bind functionality to input
void ASafeStreetCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASafeStreetCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASafeStreetCharacter::Look);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASafeStreetCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASafeStreetCharacter::StopSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ASafeStreetCharacter::StopSprint);

	}

	// Classic binding (not Enhanced Input): Escape should pause regardless of which
	// mapping context is active, so it doesn't need an Input Action asset at all.
	PlayerInputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ASafeStreetCharacter::TogglePause);
}

void ASafeStreetCharacter::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsIncapacitated)
	{
		return;
	}

	if (!OtherActor->IsA<ATrafficVehicle>())
	{
		return;
	}

	bIsIncapacitated = true;
	OnHitByVehicle();

	GetWorldTimerManager().SetTimer(GameOverTimerHandle, this, &ASafeStreetCharacter::TriggerGameOver, GameOverDelay, false);

	if (CrashMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			PlayAnimMontage(CrashMontage);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ASafeStreetCharacter::OnCrashMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, CrashMontage);
			return;
		}
	}

	// No montage assigned: don't leave the player stuck unable to move
	bIsIncapacitated = false;
}

void ASafeStreetCharacter::OnCrashMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsIncapacitated = false;
}

void ASafeStreetCharacter::TriggerGameOver()
{
	if (!GameOverWidgetClass)
	{
		return;
	}

	if (!GameOverWidget)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			GameOverWidget = CreateWidget<USafeStreetGameOverWidget>(PlayerController, GameOverWidgetClass);
			if (GameOverWidget)
			{
				GameOverWidget->AddToViewport();
			}
		}
	}

	if (GameOverWidget)
	{
		GameOverWidget->ShowGameOver();
	}
}

void ASafeStreetCharacter::TogglePause()
{
	if (bIsIncapacitated)
	{
		return;
	}

	if (!PauseWidgetClass)
	{
		return;
	}

	if (!PauseWidget)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PauseWidget = CreateWidget<USafeStreetPauseWidget>(PlayerController, PauseWidgetClass);
			if (PauseWidget)
			{
				PauseWidget->AddToViewport();
			}
		}
	}

	if (PauseWidget)
	{
		PauseWidget->TogglePause();
	}
}

void ASafeStreetCharacter::Move(const FInputActionValue& Value)
{
	if (bIsIncapacitated)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASafeStreetCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X * LookSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * LookSensitivity);
	}
}

void ASafeStreetCharacter::StartSprint(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	bIsSprinting = true;
}

void ASafeStreetCharacter::StopSprint(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	bIsSprinting = false;
}

