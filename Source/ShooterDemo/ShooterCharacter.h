// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"



UENUM(BlueprintType)
enum class ECombatState: uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FInterpLocation {
	GENERATED_BODY()

	//Scene component to use for its location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	//number of items interping to/at this scene comp location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};

UCLASS()
class SHOOTERDEMO_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//based on controller movement
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	//based on mouse X movement
	void Turn(float	Value);
	void LookUp(float Value);
	
	void FireWeapon();
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	//Set bAiming to true or false with button press
	void AimingButtonPressed();
	void AimingButtonReleased();

	void ChangeFOV(float DeltaTime);
	//set based on aiming
	void SetLookRates();

	void CalculateCrosshairSpread(float DeltaTime);

	UFUNCTION()
	void FinishCrosshairBulletFire();

	void StartCrosshairBulletFire();

	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();

	//Line trace for items under the crosshair
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	//Trace for items if overlapped item count is > 0
	void TraceForItems();

	//Spawns default weapon and equips it
	class AWeapon* SpawnDefaultWeapon();

	//takes a weapon and attaches it to the mesh
	void EquipWeapon(AWeapon* WeaponToEquip);

	//detach weapon and make it fall on the ground
	void DropWeapon();

	void SelectButtonPressed();
	void SelectButtonReleased();

	//drops currently equipped weapon and equips trace hit item
	void SwapWeapon(AWeapon* WeaponToSwap);

	void InitializeAmmoMap();

	//Check to make sure our weapon has ammo
	bool WeaponHasAmmo();

	//firte wepon functions
	void PlayFireSound();
	void SendBullet();
	void PlayGunFireMontage();
	
	void ReloadButtonPressed();

	//handle reloading of the weapon
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	//checks to see if we have ammo of the equipped ammo type
	bool CarryingAmmo();

	//called from anim blueprint with grab clip notify
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	//called from anim blueprint with release clip notify
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void CrouchButtonPressed();

	virtual void Jump() override;

	//interps capsule half height when crouching / standing
	void InterpCapsuleHalfHeight(float DeltaTime);

	void Aim();
	void StopAiming();

	void PickupAmmo(class AAmmo* Ammo);

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void InitializeInterpLocations();
	
	
private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom{nullptr};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera{nullptr};

	void MoveForward(float Value);
	void MoveRight(float Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate{0.f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate{0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound{nullptr};

	//flash particle effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash{nullptr};

	//montage for firing the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HipFireMontage{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles{nullptr};

	//smoke trail for bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles{nullptr};

	//true when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	//default camera field of view value
	float CameraDefaultFOV;

	//field of view value when zoomed in
	float CameraZoomedFOV;

	//Curent fov this frame
	float CameraCurrentFOV;

	//speed of zooming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;
	
	//turn rate while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	//when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	//turn rate when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	//when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	//sensitivity for mouse
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	//determins the spread of the crosshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier{};

	//veloctiry comp for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	//in air component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	//aim comp
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;	

	//shooting comp
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;
	
	bool bFireButtonPressed;

	// true when we can can fire, false when waiting for the timer
	bool bShouldFire;

	//rate of gun fire
	float AutomaticFireRate;

	//sets a timer between gunshots
	FTimerHandle AutoFireTimer;

	//true if we should trace every frame for items
	bool bShouldTraceForItems;

	//Number of overlapped AItems
	int8 OverlappedItemCount;

	//AItem we hit last frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;

	//currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	//set this in blueprints for the default weapon class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	//the item currently hit by trace in trace for items (could be null)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	//Distance Outward From The camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	// distance updward from the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	//Map to keep track of ammo of different ammo types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	//starting ammount of different ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items", meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items", meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	//Combat state can only fire or reload if unoccupied
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	//montage for reload animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	//transfrom of the clip when we first grab cip during reloading
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	//Scene Component to attach to the characters hand during reloading
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	//true whe crouching
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	//regular movement speed
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	//crouch movement speed
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	//current half height of capsule
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CurrentCapsuleHalfHeight;

	//half height of the capsule when not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	//half height of the capsule when crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	// Ground friction while croughing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

	//used for knowing when the aiming button is pressed
	bool bAimingButtonPressed;

	/*==============================================================*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp3;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp4;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp6;
	/*==============================================================*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

	//time to wait before we can play another pickup sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime;

	//time to wait before we can play another equip sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

	//array of AItems for inventory
	UPROPERTY(VisibleAnywhere,  BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;

	const int32 INVENTORY_CAPACITY{ 6 };
	
public:
	
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool GetAiming() const { return bAiming; }
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE bool GetCrouching() const { return bCrouching; }
	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }
	
	//ufunctions can not be inline
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const { return CrosshairSpreadMultiplier; };

	//Adds/subtracts to/from overlapped item count and updates bShouldtraceforitems
	void IncrementOverlappedItemCount(int8 Amount);

	//no longer needed AItem has interp location
	// FVector GetCameraInterpLocation();

	void GetPickupItem(AItem* Item);

	FInterpLocation GetInterpLocation(int32 Index);

	//return the index in interp locations array with the lowest item count
	int32 GetInterpLocationIndex();

	void IncrementInterpLocItemCount(int32 Index, int32 Amount);

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();
};
