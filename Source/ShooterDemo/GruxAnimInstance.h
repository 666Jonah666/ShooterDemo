// Andrei Nikitin 2022

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GruxAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERDEMO_API UGruxAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	
private:
	//lateral movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class AEnemy* Enemy;

};
