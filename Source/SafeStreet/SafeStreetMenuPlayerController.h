// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SafeStreetMenuPlayerController.generated.h"

class UUserWidget;

UCLASS()
class SAFESTREET_API ASafeStreetMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	// Widget Blueprint (child of SafeStreetMainMenuWidget) shown when the menu map starts
	UPROPERTY(EditDefaultsOnly, Category = "Menu")
	TSubclassOf<UUserWidget> MenuWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MenuWidget;
};
