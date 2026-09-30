// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetMenuGameMode.h"

#include "SafeStreetMenuPlayerController.h"

ASafeStreetMenuGameMode::ASafeStreetMenuGameMode()
{
	PlayerControllerClass = ASafeStreetMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
