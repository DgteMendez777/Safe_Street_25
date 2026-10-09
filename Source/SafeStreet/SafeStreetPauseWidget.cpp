// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetPauseWidget.h"

#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

USafeStreetPauseWidget::USafeStreetPauseWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultShowSoundFinder(
		TEXT("/Game/WidgetsFedd/Resources/Sound/oxidvideos-transition-sfx-whoosh-sound-effect-407576.oxidvideos-transition-sfx-whoosh-sound-effect-407576")
	);

	if (DefaultShowSoundFinder.Succeeded())
	{
		ShowSound = DefaultShowSoundFinder.Object;
	}
}

void USafeStreetPauseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &USafeStreetPauseWidget::HandleResumeClicked);
	}
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &USafeStreetPauseWidget::HandleRestartClicked);
	}
	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &USafeStreetPauseWidget::HandleMainMenuClicked);
	}
}

void USafeStreetPauseWidget::TogglePause()
{
	if (GetVisibility() == ESlateVisibility::Visible)
	{
		ResumeGame();
	}
	else
	{
		ShowPause();
	}
}

void USafeStreetPauseWidget::ShowPause()
{
	SetVisibility(ESlateVisibility::Visible);

	if (ShowSound)
	{
		UGameplayStatics::PlaySound2D(this, ShowSound);
	}

	UGameplayStatics::SetGamePaused(GetWorld(), true);

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
	}
}

void USafeStreetPauseWidget::ResumeGame()
{
	SetVisibility(ESlateVisibility::Collapsed);

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}
}

void USafeStreetPauseWidget::HandleResumeClicked()
{
	ResumeGame();
}

void USafeStreetPauseWidget::HandleRestartClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void USafeStreetPauseWidget::HandleMainMenuClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (!MainMenuMap.IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuMap);
	}
}
