// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SafeStreetFeedbackWidget.generated.h"

class UTexture2D;
class UImage;
class USoundBase;

UCLASS()
class SAFESTREET_API USafeStreetFeedbackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USafeStreetFeedbackWidget(const FObjectInitializer& ObjectInitializer);

	// Shows the full-screen image mapped to this class name (if any), sliding it in from
	// off-screen and pausing gameplay input until the player dismisses it with any key.
	// Call this from ASafeStreetAIClient::OnFeedbackDetected.
	UFUNCTION(BlueprintCallable, Category = "Feedback")
	void ShowFeedbackFor(const FString& ClassName);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// Name an Image widget exactly "FeedbackImage", filling the screen, in the Widget Blueprint
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> FeedbackImage;

	// One full infographic image per AI class name (must match the exact class_name string)
	UPROPERTY(EditAnywhere, Category = "Feedback")
	TMap<FString, TObjectPtr<UTexture2D>> FeedbackImages;

	// How far off-screen (pixels) the image starts/ends during the slide
	UPROPERTY(EditAnywhere, Category = "Feedback", meta = (ClampMin = "0.0"))
	float SlideDistance = 1920.f;

	// Seconds the slide-in/slide-out takes
	UPROPERTY(EditAnywhere, Category = "Feedback", meta = (ClampMin = "0.01"))
	float SlideDuration = 0.5f;

	// Played once when the feedback appears; leave empty for silent
	UPROPERTY(EditAnywhere, Category = "Feedback")
	TObjectPtr<USoundBase> ShowSound;

private:
	void HideFeedback();

	enum class ESlideState : uint8
	{
		Hidden,
		SlidingIn,
		Shown,
		SlidingOut
	};

	ESlideState SlideState = ESlideState::Hidden;
	float SlideTime = 0.f;
};
