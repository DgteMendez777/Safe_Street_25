// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SafeStreetMainMenuWidget.generated.h"

class UWorld;
class UWidget;

UCLASS()
class SAFESTREET_API USafeStreetMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Fired once when the player presses any key, for extras done in the Widget Blueprint (sound, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
	void OnStartRequested();

	// The "press any key" text. Name a widget exactly "PromptText" in the Widget Blueprint to animate it.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> PromptText;

	// Map opened after the key press
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu")
	TSoftObjectPtr<UWorld> GameMap;

	// Seconds between the key press and the map load; the whole menu fades out during this time
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu", meta = (ClampMin = "0.0"))
	float LoadDelay = 1.f;

	// Seconds the whole menu takes to fade in when it appears
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Prompt Animation", meta = (ClampMin = "0.0"))
	float FadeInDuration = 2.f;

	// How far (in pixels) the prompt drifts up and down
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Prompt Animation", meta = (ClampMin = "0.0"))
	float FloatAmplitude = 10.f;

	// Seconds for one full up-and-down cycle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Prompt Animation", meta = (ClampMin = "0.1"))
	float FloatPeriod = 3.f;

	// Prompt opacity at the dimmest and brightest points of the pulse
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Prompt Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinPromptOpacity = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Prompt Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxPromptOpacity = 1.f;

private:
	void StartGame();
	void OpenGameMap();

	bool bStartRequested = false;

	float TimeSinceConstruct = 0.f;
	float TimeSinceStartRequested = 0.f;

	FTimerHandle LoadTimerHandle;
};
