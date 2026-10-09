// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetFeedbackWidget.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

USafeStreetFeedbackWidget::USafeStreetFeedbackWidget(const FObjectInitializer& ObjectInitializer)
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

void USafeStreetFeedbackWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Collapsed);
}

void USafeStreetFeedbackWidget::ShowFeedbackFor(const FString& ClassName)
{
	if (SlideState != ESlideState::Hidden)
	{
		return;
	}

	TObjectPtr<UTexture2D>* FoundImage = FeedbackImages.Find(ClassName);
	if (!FoundImage || !*FoundImage || !FeedbackImage)
	{
		return;
	}

	FeedbackImage->SetBrushFromTexture(*FoundImage);

	if (ShowSound)
	{
		UGameplayStatics::PlaySound2D(this, ShowSound);
	}

	SlideState = ESlideState::SlidingIn;
	SlideTime = 0.f;

	SetVisibility(ESlateVisibility::Visible);
	SetRenderTranslation(FVector2D(-SlideDistance, 0.f));

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
	}
}

void USafeStreetFeedbackWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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
			SlideState = ESlideState::Hidden;
			SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

FReply USafeStreetFeedbackWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (SlideState == ESlideState::Shown)
	{
		HideFeedback();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void USafeStreetFeedbackWidget::HideFeedback()
{
	SlideState = ESlideState::SlidingOut;
	SlideTime = 0.f;

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}
}
