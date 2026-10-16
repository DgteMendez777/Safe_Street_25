// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SafeStreetIntroWidget.generated.h"

class UTexture2D;
class UImage;
class USoundBase;

UCLASS()
class SAFESTREET_API USafeStreetIntroWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USafeStreetIntroWidget(const FObjectInitializer& ObjectInitializer);

	// Starts the intro: shows each page in order (Space advances) and pauses the game until the last one closes
	UFUNCTION(BlueprintCallable, Category = "Intro")
	void ShowIntro();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// Name an Image widget exactly "IntroImage", filling the screen, in the Widget Blueprint
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> IntroImage;

	// Pages shown in order. The first one (la_calle) is set by default; add the next ones in Class Defaults.
	UPROPERTY(EditAnywhere, Category = "Intro")
	TArray<TObjectPtr<UTexture2D>> IntroImages;

	// How far off-screen (pixels) each page starts/ends during the slide
	UPROPERTY(EditAnywhere, Category = "Intro", meta = (ClampMin = "0.0"))
	float SlideDistance = 1920.f;

	// Seconds each slide-in/slide-out takes
	UPROPERTY(EditAnywhere, Category = "Intro", meta = (ClampMin = "0.01"))
	float SlideDuration = 0.5f;

	// Played each time a page appears; leave empty for silent
	UPROPERTY(EditAnywhere, Category = "Intro")
	TObjectPtr<USoundBase> ShowSound;

	// Welcome voice played right after the player closes the page at WelcomeAfterPage; leave empty for silent
	UPROPERTY(EditAnywhere, Category = "Intro")
	TObjectPtr<USoundBase> WelcomeSound;

	// Voice played right after WelcomeSound finishes (the "you just left school, catch the bus" objective)
	UPROPERTY(EditAnywhere, Category = "Intro")
	TObjectPtr<USoundBase> ObjectiveSound;

	// Zero-based index of the page after which the welcome voice plays (0 = the first page)
	UPROPERTY(EditAnywhere, Category = "Intro", meta = (ClampMin = "0"))
	int32 WelcomeAfterPage = 0;

private:
	void ShowPage(int32 Index);
	void FinishIntro();

	UFUNCTION()
	void OnWelcomeFinished();

	enum class ESlideState : uint8
	{
		Hidden,
		SlidingIn,
		Shown,
		SlidingOut
	};

	ESlideState SlideState = ESlideState::Hidden;
	float SlideTime = 0.f;
	int32 PageIndex = 0;
};
