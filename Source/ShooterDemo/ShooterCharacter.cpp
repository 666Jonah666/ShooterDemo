// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "Ammo.h"
#include "DrawDebugHelpers.h"
#include "Item.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
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
	CameraZoomedFOV(25.f),
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
	MouseAimingTurnRate(0.6f),
	MouseAimingLookUpRate(0.6f),
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
	bShouldTraceForItems(false),
	//camera interp location variables
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
	//starting ammo amounts
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	//Combat variables
	CombatState(ECombatState::ECS_Unoccupied),
	bCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(300.f),
	CurrentCapsuleHalfHeight(0.f),
	StandingCapsuleHalfHeight(88.f),
	CrouchingCapsuleHalfHeight(44.f),
	BaseGroundFriction(2.f),
	CrouchingGroundFriction(100.f),
	bAimingButtonPressed(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Attach SpringArm; pulls in towards a character if there is a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 240.f; //arm length / camera distance
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

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	//Create interpolation components
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponInterpolationComponent"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent1"));
	InterpComp1->SetupAttachment(GetFollowCamera());
	
	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent2"));
	InterpComp2->SetupAttachment(GetFollowCamera());

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent3"));
	InterpComp3->SetupAttachment(GetFollowCamera());

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent4"));
	InterpComp4->SetupAttachment(GetFollowCamera());

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent5"));
	InterpComp5->SetupAttachment(GetFollowCamera());

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
	
}
 
// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera) {
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	//Spawn default weapon and equip it
	EquipWeapon(SpawnDefaultWeapon());

	InitializeAmmoMap();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
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
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased() {
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer() {
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(
	AutoFireTimer,
	this,
	&AShooterCharacter::AutoFireReset,
	AutomaticFireRate);
	
}

void AShooterCharacter::AutoFireReset() {
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo()) {
		if(bFireButtonPressed) {
			FireWeapon();
		}
	} else {
		//reload weapon
		ReloadWeapon();
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
            		TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);
            		if (TraceHitItem && TraceHitItem->GetPickupWidget()) {
            			//Show items pickup widget
            			TraceHitItem->GetPickupWidget()->SetVisibility(true);
            		}

            		//we hit an AItem last frame
                    if (TraceHitItemLastFrame) {
	                    if (TraceHitItem != TraceHitItemLastFrame) {
							//we are hitting different hit item then last frame or its null
	                    	//then we need to make hit item last frame invisible
	                    	TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	                    }
                    }

            		TraceHitItemLastFrame = TraceHitItem;

            	} 
	} else if (TraceHitItemLastFrame) {
		//no longer overlapping any items
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon() {

	//check default weapon class
	if(DefaultWeaponClass) {
		//spawn weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip) {
	if (WeaponToEquip) {

		//get hand socket from skeletal mesh
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));

		if (HandSocket) {
			//attach weapon to the hand socket RightHandSocket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		//set equipped weapon to the newly spawned weapon
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
		
		
	}
}

void AShooterCharacter::DropWeapon() {
	if (EquippedWeapon) {
		FDetachmentTransformRules DetachmentTransformRules{EDetachmentRule::KeepWorld, true};
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed() {
	if(TraceHitItem) {
		TraceHitItem->StartItemCurve(this);
 	} 
}

void AShooterCharacter::SelectButtonReleased() {
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap) {
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap() {
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);

}

bool AShooterCharacter::WeaponHasAmmo() {
	if(!EquippedWeapon) {
		return false;
	}

	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound() {
	//play fire sound
	if (FireSound) {
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SendBullet() {
	//send bullet 
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	
	if (BarrelSocket) {
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

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
}

void AShooterCharacter::PlayGunFireMontage() {
	//play hip fire montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage) {
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed() {
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon() {
	if(CombatState != ECombatState::ECS_Unoccupied) {
		return;
	}

	if (!EquippedWeapon) {
		return;
	}

	//do we have ammo of the correct type? && do we have empty space in the magazine?
	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull()) {
		if (bAiming) {
			StopAiming();
		}		
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (ReloadMontage && AnimInstance) {
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
	
}

void AShooterCharacter::FinishReloading() {

	//update combat state
	CombatState = ECombatState::ECS_Unoccupied;

	if(bAimingButtonPressed) {
		Aim();
	}
	
	if(!EquippedWeapon) {
		return;
	}

	const EAmmoType AmmoType{EquippedWeapon->GetAmmoType()};
	
	//UpdateAmmoMap	
	if (AmmoMap.Contains(AmmoType)) {
		//amount of ammo of carried weapon ammo type
		int32 CarriedAmmo = AmmoMap[AmmoType];

		//space left in the magazine of equipped weapon
		const int32 MagEmptySpace{EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo()};
		if (MagEmptySpace > CarriedAmmo) {
			//reload the magazine with all the ammo we are carrying, because ammo we have is less then max ammo in magazine
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		} else {
			//fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
	
}

bool AShooterCharacter::CarryingAmmo() {
	if (!EquippedWeapon) {
		return false;
	}

	EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType)) {
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip() {
	
	if (!EquippedWeapon || !HandSceneComponent) {
		return;
	}

	//index for the clip bone on the equipped weapon
	int32 ClipBoneIndex{EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName())};
	//store transform of the clip
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
	
}

void AShooterCharacter::ReleaseClip() {

	EquippedWeapon->SetMovingClip(false);

}

void AShooterCharacter::CrouchButtonPressed() {
	if (!GetCharacterMovement()->IsFalling()) {
		//toggle bCrouching
		bCrouching = !bCrouching;
	}

	GetCharacterMovement()->MaxWalkSpeed = bCrouching ? CrouchMovementSpeed : BaseMovementSpeed;

	if(bCrouching) {
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	} else {
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
	
}

void AShooterCharacter::Jump() {
	Super::Jump();

	if(bCrouching) {
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	} else {
		ACharacter::Jump();
	}
}

// Called every frame
void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime) {
	float TargetCapsuleHalfHeight{bCrouching ? CrouchingCapsuleHalfHeight : StandingCapsuleHalfHeight};

	const float InterpHalfHeight{FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f)};
	//negative value if crouching; positive if standing
	const float DeltaCapsuleHalfHeight{InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
	const FVector MeshOffset{0.f, 0.f, -DeltaCapsuleHalfHeight};
	GetMesh()->AddLocalOffset(MeshOffset);
	
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::Aim() {
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming() {
	bAiming = false;
	if (!bCrouching) {
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;	
	}
}



void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ChangeFOV(DeltaTime);
	
	//change sensitivity if aiming
	SetLookRates();
	
	CalculateCrosshairSpread(DeltaTime);

	//Check overlapped item count then trace for items
	TraceForItems();

	//interpolate the capsule half height based on crouching / standing
	InterpCapsuleHalfHeight(DeltaTime);
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

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

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

	if (!EquippedWeapon) {
		return;
	}

	if (CombatState != ECombatState::ECS_Unoccupied) {
		return;
	}

	if (WeaponHasAmmo()) {
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();
		EquippedWeapon->DecrementAmmo();
		StartFireTimer();
		//Start bullet fire timer for crosshairs
		StartCrosshairBulletFire();
	}


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
	bAimingButtonPressed = true;
	if(CombatState != ECombatState::ECS_Reloading) {
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased() {
	bAimingButtonPressed = false;
	StopAiming();
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

FVector AShooterCharacter::GetCameraInterpLocation() {
	const FVector CameraWorldLocation{FollowCamera->GetComponentLocation()};
	const FVector CameraForward{FollowCamera->GetForwardVector()};

	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
}

void AShooterCharacter::GetPickupItem(AItem* Item) {

	if (Item->GetEquipSound()) {
		UGameplayStatics::PlaySound2D(this, Item->GetEquipSound());
	}

	AWeapon* Weapon = Cast<AWeapon>(Item);
	if (Weapon) {
		SwapWeapon(Weapon);
	}

	AAmmo* Ammo = Cast<AAmmo>(Item);
	if (Ammo) {
		PickupAmmo(Ammo);
	}
}


void AShooterCharacter::PickupAmmo(AAmmo* Ammo) {

	//check to see if ammo map contains ammo type
	if(AmmoMap.Find(Ammo->GetAmmoType())) {
		//get amount of ammo
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		//set the amount of ammo in the map for this type
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if(EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType()) {
		//check to see if the gun is empty
		if(EquippedWeapon->GetAmmo() == 0) {
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
	
}


