// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"

#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),
	bAiming(false),
	CharacterYaw(0.f),
	CharacterYawLastFrame(0.f),
	RootYawOffset(0.f),
	Pitch(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip)
{
	
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime) {
	if (!ShooterCharacter) {
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if(ShooterCharacter) {

		bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
		
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

		FRotator AimRotation{ShooterCharacter->GetBaseAimRotation()};
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if(ShooterCharacter->GetVelocity().Size() > 0.f) {
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		bAiming = ShooterCharacter->GetAiming();

		if (bReloading) {
			OffsetState = EOffsetState::EOS_Reloading;
		} else if (bIsInAir) {
			OffsetState = EOffsetState::EOS_InAir;
		} else if (ShooterCharacter->GetAiming()) {
			OffsetState = EOffsetState::EOS_Aiming;
		} else {
			OffsetState = EOffsetState::EOS_Hip;
		}
	}

	TurnInPlace();
	
}

void UShooterAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace() {
	if (!ShooterCharacter) {
		return;
	}

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

	if(Speed > 0 || bIsInAir) {
		//dont want to turn in place when character is moving
		RootYawOffset = 0.f;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		CharacterYawLastFrame = CharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	} else {
		CharacterYawLastFrame = CharacterYaw;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float YawDelta { CharacterYaw - CharacterYawLastFrame };

		//root yaw offset updated and clamped to [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

		const float Turning{ GetCurveValue(TEXT("Turning")) };

		if (Turning > 0) {
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("CurveRotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			//rootYawOffset> 0, -> turning left; root yaw offset < 0 -> turn right
			(RootYawOffset > 0) ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset { FMath::Abs(RootYawOffset) };
			if(ABSRootYawOffset > 90.f) {
				const float YawExcess{ ABSRootYawOffset - 90.f};
				(RootYawOffset > 0) ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
	}
}


