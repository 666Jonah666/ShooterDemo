// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"
/**
 * 
 */
UCLASS()
class SHOOTERDEMO_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void Tick(float DeltaSeconds) override;

protected:
	void StopFalling();
private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	//ammo count for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;
	
public:
	FORCEINLINE int32 GetAmmo() const { return Ammo; } 

	//Adds an impulse to the weapon
	void ThrowWeapon();

	//called from cahracter class when firing weapon
	void DecrementAmmo();
};
