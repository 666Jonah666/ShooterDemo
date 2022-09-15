// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "DrawDebugHelpers.h"
#include "Item.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	//Base rates for turning/looking up
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	//true when aiming
	bAiming(false),
	//camera field of view values
	CameraDefaultFOV(0.f), //set in begin play
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),
	// Turn rates for aiming/not aiming
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	//Mouse Look sensitivity scale factors
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),
	//Crosshair spread factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	//Bullet fire timer variables
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	//Automatic gun fire rate
	bFireButtonPressed(false),
	bShouldFire(true),
	AutomaticFireRate(0.1f),
	//item trace variables
	bShouldTraceForItems(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Attach SpringArm; pulls in towards a character if there is a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; //arm length / camera distance
	CameraBoom->bUsePawnControlRotation = true; //rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//Attach Camera to SpringArm
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; //camera does not rotate when we rotate arm

	//don't rotate character with camera. rotate camera only instead
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true; //might be overrited in blueprint 
	bUseControllerRotationRoll = false;

	//Configure Character Movement
	/* uncheck use controller ...
	   check orient rotation ...
	   in blueprints   */
	
	GetCharacterMovement()->bOrientRotationToMovement = false; //character moves in the direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera) {
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
}

void AShooterCharacter::StartCrosshairBulletFire() {
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer,
		this,
		&AShooterCharacter::FinishCrosshairBulletFire,
		ShootTimeDuration);
}


void AShooterCharacter::FireButtonPressed() {
	bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased() {
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer() {
	if (bShouldFire) {
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(
			AutoFireTimer,
			this,
			&AShooterCharacter::AutoFireReset,
			AutomaticFireRate);
	}
}

void AShooterCharacter::AutoFireReset() {
	bShouldFire = true;
	if(bFireButtonPressed) {
		StartFireTimer();
	}
	
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation) {

	//GetViewPortSize
	FVector2D ViewPortSize{FVector2D::ZeroVector};
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	//get screen space location of crosshairs
	FVector2D CrosshairLocation{ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f};
	//subtract because we move location of crosshair by 50 units
	// CrosshairLocation.Y -= 50.f;

	FVector CrosshairWorldPosition{FVector::ZeroVector};
	FVector CrosshairWorldDirection{FVector::ZeroVector};

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld) {
		//Trace from crosshair world location outworld
		const FVector Start{CrosshairWorldPosition};
		const FVector End{Start + CrosshairWorldDirection * 50'000.f};
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(
			OutHitResult,
			Start,
			End,
			ECC_Visibility);

		if(OutHitResult.bBlockingHit) {
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	
	return false;
}

void AShooterCharacter::TraceForItems() {
	if(bShouldTraceForItems) {
    		FHitResult ItemTraceResult;
            	FVector HitLocation{FVector::ZeroVector};
            	TraceUnderCrosshairs(ItemTraceResult, HitLocation);
            
            	if (ItemTraceResult.bBlockingHit) {
            		//for ue5 use ItemTraceResult.GetActor()
            		AItem* HitItem = Cast<AItem>(ItemTraceResult.Actor);
            		if (HitItem && HitItem->GetPickupWidget()) {
            			//Show items pickup widget
            			HitItem->GetPickupWidget()->SetVisibility(true);
            		}

            		//we hit an AItem last frame
                    if (TraceHitItemLastFrame) {
	                    if (HitItem != TraceHitItemLastFrame) {
							//we are hitting different hit item then last frame or its null
	                    	//then we need to make hit item last frame invisible
	                    	TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	                    }
                    }

            		TraceHitItemLastFrame = HitItem;

            	} 
	} else if (TraceHitItemLastFrame) {
		//no longer overlapping any items
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ChangeFOV(DeltaTime);

	//change sensitivity if aiming
	SetLookRates();
	
	CalculateCrosshairSpread(DeltaTime);

	//Check overlapped item count then trace for items
	TraceForItems();

}

void AShooterCharacter::SetLookRates() {
	if(bAiming) {
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	} else {
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime) {

	FVector2D WalkSpeedRange{0.f, 600.f};
	FVector2D VelocityMultiplierRange{0.f, 1.f};
	FVector Velocity{GetVelocity()};
	Velocity.Z = 0.f;

	//velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	//calculate crosshair in air factor
	if(GetCharacterMovement()->IsFalling()) {
		//spread the crosshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	} else {
		//character is on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	//if aiming spread crosshair
	if (bAiming) {
		//spread in
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.5f, DeltaTime, 30.f);
	} else {
		//spread out to normal
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	//True 0.05 sec after firing
	if (bFiringBullet) {
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	} else {
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}
	
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor + CrosshairShootingFactor;
	

}

void AShooterCharacter::FinishCrosshairBulletFire() {
	bFiringBullet = false;
}

void AShooterCharacter::ChangeFOV(float DeltaTime) {
	//set current camera field of view depending on zooming
	if(bAiming) {
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	} else {
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
	
}


// Called to bind functionality to input

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	
	
}

void AShooterCharacter::MoveForward(float Value) {
	if (Controller && Value != 0.f) {
		const FRotator Rotation{Controller->GetControlRotation()};
		const FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate) {
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}
void AShooterCharacter::LookUpAtRate(float Rate) {
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());	
}

void AShooterCharacter::Turn(float Value) {

	float TurnScaleFactor;
	if(bAiming) {
		TurnScaleFactor = MouseAimingTurnRate;
	} else {
		TurnScaleFactor = MouseHipTurnRate;
	}

	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value) {
	float LookUpScaleFactor;
	if(bAiming) {
		LookUpScaleFactor = MouseAimingLookUpRate;
	} else {
		LookUpScaleFactor = MouseHipLookUpRate;
	}

	AddControllerPitchInput(Value * LookUpScaleFactor);
}



void AShooterCharacter::FireWeapon() {
	if (FireSound) {
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	
	if (BarrelSocket) {
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd{FVector::ZeroVector};
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd) {
			//spawn impact particles after updating beam end point
			if(ImpactParticles) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}

			if (BeamParticles) {
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					BeamParticles,
					SocketTransform);
					
				//"Target" particle system for beam behaviour (vector) which we take from P_SmokeTrail
				if (Beam) {
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage) {
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	//Start bullet fire timer for crosshairs
	StartCrosshairBulletFire();
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation) {

	//Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit{TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation)};

	if (bCrosshairHit) {
		//Tentative beam location -- still need to trace from gun
		OutBeamLocation = CrosshairHitResult.Location;
	} else { // no hit
		//OutBeamLocation is the end location of the line trace
	}

	//Perform trace from gun barrel
	FHitResult WeaponTraceHit{};
	const FVector WeaponTracStart{MuzzleSocketLocation};
	const FVector StartToEnd{OutBeamLocation - MuzzleSocketLocation};
	const FVector WeaponTraceEnd{MuzzleSocketLocation + StartToEnd * 1.25f};
	GetWorld()->LineTraceSingleByChannel(
		WeaponTraceHit,
		WeaponTracStart,
		WeaponTraceEnd,
		ECC_Visibility);

	//object between barrel and beam end point
	if (WeaponTraceHit.bBlockingHit) {
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}

	return false;
	
	
}

void AShooterCharacter::AimingButtonPressed() {
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased() {
	bAiming = false;
}

void AShooterCharacter::MoveRight(float Value) {
	if (Controller && Value != 0.f) {
		const FRotator Rotation{Controller->GetControlRotation()};
		const FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount) {
	if (OverlappedItemCount + Amount <= 0) {
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	} else {
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}


