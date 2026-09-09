// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeStreetAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

void USafeStreetAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (const APawn* OwningPawn = TryGetPawnOwner())
	{
		Speed = OwningPawn->GetVelocity().Size();

		if (const ACharacter* OwningCharacter = Cast<ACharacter>(OwningPawn))
		{
			bIsInAir = OwningCharacter->GetCharacterMovement()->IsFalling();
		}
	}
}

