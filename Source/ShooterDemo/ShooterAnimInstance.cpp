// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"

#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UShooterAnimInstance::UpdateAnimationPropeties(float DeltaTime) {
	if (!ShooterCharacter) {
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if(ShooterCharacter) {
		//Get the lateral speed of character from velocity
		FVector Velocity{ShooterCharacter->GetVelocity()};
		Velocity.Z = 0;
		Speed = Velocity.Size();

		//Is the character in the air
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		//is the character Accelerating
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f) {
			bIsAccelerating = true;
		} else {
			bIsAccelerating = false;
		}
	}
	
}

void UShooterAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}


