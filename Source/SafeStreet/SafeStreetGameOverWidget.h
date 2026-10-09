// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SafeStreetGameOverWidget.generated.h"

class UButton;
class UWorld;
class USoundBase;

UCLASS()
class SAFESTREET_API USafeStreetGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USafeStreetGameOverWidget(const FObjectInitializer& ObjectInitializer);

	// Shows the screen and pauses the game; call this when the player loses
	UFUNCTION(BlueprintCallable, Category = "Game Over")
	void ShowGameOver();

protected:
	virtual void NativeConstruct() override;

	// Name buttons exactly these in the Widget Blueprint to wire them automatically
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainMenuButton;

	// Map opened by the Main Menu button
	UPROPERTY(EditAnywhere, Category = "Game Over")
	TSoftObjectPtr<UWorld> MainMenuMap;

	// Played once when the Game Over screen appears; leave empty for silent
	UPROPERTY(EditAnywhere, Category = "Game Over")
	TObjectPtr<USoundBase> ShowSound;

private:
	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleMainMenuClicked();
};
