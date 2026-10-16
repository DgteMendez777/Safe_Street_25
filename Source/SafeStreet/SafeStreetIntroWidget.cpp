// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetIntroWidget.h"

#include "Components/AudioComponent.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

USafeStreetIntroWidget::USafeStreetIntroWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultShowSoundFinder(
		TEXT("/Game/WidgetsFedd/Resources/Sound/oxidvideos-transition-sfx-whoosh-sound-effect-407576.oxidvideos-transition-sfx-whoosh-sound-effect-407576")
	);

	if (DefaultShowSoundFinder.Succeeded())
	{
		ShowSound = DefaultShowSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> WelcomeSoundFinder(
		TEXT("/Game/WidgetsFedd/Resources/Sound/BienvenidaSafeStreet.BienvenidaSafeStreet")
	);

	if (WelcomeSoundFinder.Succeeded())
	{
		WelcomeSound = WelcomeSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> ObjectiveSoundFinder(
		TEXT("/Game/WidgetsFedd/Resources/Sound/SalirColegio.SalirColegio")
	);

	if (ObjectiveSoundFinder.Succeeded())
	{
		ObjectiveSound = ObjectiveSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> LaCalleFinder(
		TEXT("/Game/WidgetsFedd/Resources/la_calle.la_calle")
	);

	if (LaCalleFinder.Succeeded())
	{
		IntroImages.Add(LaCalleFinder.Object);
	}
}

void USafeStreetIntroWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Collapsed);
}

void USafeStreetIntroWidget::ShowIntro()
{
	if (SlideState != ESlideState::Hidden || IntroImages.IsEmpty() || !IntroImage)
	{
		return;
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

	SetVisibility(ESlateVisibility::Visible);
	ShowPage(0);
}

void USafeStreetIntroWidget::ShowPage(int32 Index)
{
	PageIndex = Index;

	IntroImage->SetBrushFromTexture(IntroImages[PageIndex]);

	if (ShowSound)
	{
		UGameplayStatics::PlaySound2D(this, ShowSound);
	}

	SlideState = ESlideState::SlidingIn;
	SlideTime = 0.f;
	SetRenderTranslation(FVector2D(-SlideDistance, 0.f));
}

void USafeStreetIntroWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (SlideState == ESlideState::Hidden || SlideState == ESlideState::Shown)
	{
		return;
	}

	SlideTime += InDeltaTime;

	const float Alpha = SlideDuration > 0.f ? FMath::Clamp(SlideTime / SlideDuration, 0.f, 1.f) : 1.f;
	const float Eased = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f);

	if (SlideState == ESlideState::SlidingIn)
	{
		SetRenderTranslation(FVector2D(FMath::Lerp(-SlideDistance, 0.f, Eased), 0.f));

		if (Alpha >= 1.f)
		{
			SlideState = ESlideState::Shown;
		}
	}
	else if (SlideState == ESlideState::SlidingOut)
	{
		SetRenderTranslation(FVector2D(FMath::Lerp(0.f, SlideDistance, Eased), 0.f));

		if (Alpha >= 1.f)
		{
			if (WelcomeSound && PageIndex == WelcomeAfterPage)
			{
				if (UAudioComponent* WelcomeAudio = UGameplayStatics::SpawnSound2D(this, WelcomeSound))
				{
					WelcomeAudio->OnAudioFinished.AddDynamic(this, &USafeStreetIntroWidget::OnWelcomeFinished);
				}
			}

			if (PageIndex + 1 < IntroImages.Num())
			{
				ShowPage(PageIndex + 1);
			}
			else
			{
				FinishIntro();
			}
		}
	}
}

FReply USafeStreetIntroWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (SlideState == ESlideState::Shown && InKeyEvent.GetKey() == EKeys::SpaceBar)
	{
		SlideState = ESlideState::SlidingOut;
		SlideTime = 0.f;
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void USafeStreetIntroWidget::OnWelcomeFinished()
{
	if (ObjectiveSound)
	{
		UGameplayStatics::PlaySound2D(this, ObjectiveSound);
	}
}

void USafeStreetIntroWidget::FinishIntro()
{
	SlideState = ESlideState::Hidden;
	SetVisibility(ESlateVisibility::Collapsed);

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}
}
