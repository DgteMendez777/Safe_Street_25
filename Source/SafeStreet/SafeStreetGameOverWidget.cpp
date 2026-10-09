// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetGameOverWidget.h"

#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

USafeStreetGameOverWidget::USafeStreetGameOverWidget(const FObjectInitializer& ObjectInitializer)
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

void USafeStreetGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);

	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &USafeStreetGameOverWidget::HandleRestartClicked);
	}
	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &USafeStreetGameOverWidget::HandleMainMenuClicked);
	}
}

void USafeStreetGameOverWidget::ShowGameOver()
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

void USafeStreetGameOverWidget::HandleRestartClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void USafeStreetGameOverWidget::HandleMainMenuClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (!MainMenuMap.IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuMap);
	}
}
