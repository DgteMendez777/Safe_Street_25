// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetMainMenuWidget.h"

#include "Components/Widget.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

USafeStreetMainMenuWidget::USafeStreetMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultAmbienceSoundFinder(
		TEXT("/Game/WidgetsFedd/Resources/Sound/ambiencemenu.ambiencemenu")
	);

	if (DefaultAmbienceSoundFinder.Succeeded())
	{
		AmbienceSound = DefaultAmbienceSoundFinder.Object;
	}
}

void USafeStreetMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	SetRenderOpacity(0.f);

	if (AmbienceSound)
	{
		UGameplayStatics::PlaySound2D(this, AmbienceSound, AmbienceVolume);
	}
}

void USafeStreetMainMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TimeSinceConstruct += InDeltaTime;

	float MenuOpacity = FadeInDuration > 0.f ? FMath::Clamp(TimeSinceConstruct / FadeInDuration, 0.f, 1.f) : 1.f;

	if (bStartRequested)
	{
		TimeSinceStartRequested += InDeltaTime;
		const float FadeOut = LoadDelay > 0.f ? FMath::Clamp(TimeSinceStartRequested / LoadDelay, 0.f, 1.f) : 1.f;
		MenuOpacity *= 1.f - FadeOut;
	}

	SetRenderOpacity(MenuOpacity);

	if (PromptText)
	{
		const float Phase = TimeSinceConstruct * 2.f * PI / FloatPeriod;
		const float Wave = FMath::Sin(Phase);
		const float Pulse = (Wave + 1.f) * 0.5f;

		PromptText->SetRenderTranslation(FVector2D(0.f, -Wave * FloatAmplitude));
		PromptText->SetRenderOpacity(FMath::Lerp(MinPromptOpacity, MaxPromptOpacity, Pulse));
	}
}

FReply USafeStreetMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	StartGame();
	return FReply::Handled();
}

FReply USafeStreetMainMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	StartGame();
	return FReply::Handled();
}

void USafeStreetMainMenuWidget::StartGame()
{
	if (bStartRequested)
	{
		return;
	}
	bStartRequested = true;
	TimeSinceStartRequested = 0.f;

	OnStartRequested();

	if (LoadDelay <= 0.f)
	{
		OpenGameMap();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(LoadTimerHandle, this, &USafeStreetMainMenuWidget::OpenGameMap, LoadDelay, false);
}

void USafeStreetMainMenuWidget::OpenGameMap()
{
	if (GameMap.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: GameMap is not set, nothing to open."));
		bStartRequested = false;
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameMap);
}
