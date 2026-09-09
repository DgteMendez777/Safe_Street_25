// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetGameMode.h"

#include "SafeStreetCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASafeStreetGameMode::ASafeStreetGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Imput/BP_SafeStreetCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

