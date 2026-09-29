// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetMenuPlayerController.h"

#include "Blueprint/UserWidget.h"

void ASafeStreetMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController() || !MenuWidgetClass)
	{
		return;
	}

	MenuWidget = CreateWidget<UUserWidget>(this, MenuWidgetClass);
	if (!MenuWidget)
	{
		return;
	}

	MenuWidget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	bShowMouseCursor = false;
}
