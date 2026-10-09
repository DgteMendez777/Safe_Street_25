// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SafeStreetPauseWidget.generated.h"

class UButton;
class UWorld;
class USoundBase;

UCLASS()
class SAFESTREET_API USafeStreetPauseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USafeStreetPauseWidget(const FObjectInitializer& ObjectInitializer);

	// Shows/hides the menu and pauses/resumes the game; call this when the player presses the pause key
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void TogglePause();

protected:
	virtual void NativeConstruct() override;

	// Name buttons exactly these in the Widget Blueprint to wire them automatically
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainMenuButton;

	// Map opened by the Main Menu button
	UPROPERTY(EditAnywhere, Category = "Pause")
	TSoftObjectPtr<UWorld> MainMenuMap;

	// Played once when the pause menu appears; leave empty for silent
	UPROPERTY(EditAnywhere, Category = "Pause")
	TObjectPtr<USoundBase> ShowSound;

private:
	UFUNCTION()
	void HandleResumeClicked();

	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleMainMenuClicked();

	void ShowPause();
	void ResumeGame();
};
