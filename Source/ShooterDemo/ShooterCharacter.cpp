// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
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
	MouseAimingLookUpRate(0.2f)
	
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

// Called every frame

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ChangeFOV(DeltaTime);

	//change sensitivity if aiming
	SetLookRates();
	
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

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireWeapon);

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
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation) {
	//Get current size of the viewport
	FVector2D ViewPortSize{FVector2D::ZeroVector};
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	//get screen space location of crosshairs
	FVector2D CrosshairLocation{ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f};
	//subtract because we move location of crosshair by 50 units
	CrosshairLocation.Y -= 50.f;

	FVector CrosshairWorldPosition{FVector::ZeroVector};
	FVector CrosshairWorldDirection{FVector::ZeroVector};

	//get world position and direction of crosshairs
	//having crosshairs position in the world and direction pointing outward away from the camera
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	//performing linetrace from this position and direction across world direction
	// was projection successful?
	if (bScreenToWorld) {
		FHitResult ScreenTraceHit{};
		const FVector Start{CrosshairWorldPosition};
		const FVector End{CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f};

		//Set beam end point to line end point so if bullet doesnt hit anywhere we particle will be till the end
		OutBeamLocation = End;

		//trace outward from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECC_Visibility);

		// if hit smth
		if(ScreenTraceHit.bBlockingHit) {
			//smoke trail till end location
			OutBeamLocation = ScreenTraceHit.Location;

			//perform line trace from barrel gun
			FHitResult WeaponTraceHit{};
			const FVector WeaponTracStart{MuzzleSocketLocation};
			const FVector WeaponTraceEnd{OutBeamLocation};
			GetWorld()->LineTraceSingleByChannel(
				WeaponTraceHit,
				WeaponTracStart,
				WeaponTraceEnd,
				ECC_Visibility);

			//object between barrel and beam end point
			if (WeaponTraceHit.bBlockingHit) {
				OutBeamLocation = WeaponTraceHit.Location;
			}
		}
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


