// Andrei Nikitin 2022


#include "GruxAnimInstance.h"

#include "Enemy.h"

void UGruxAnimInstance::UpdateAnimationProperties(float DeltaTime) {
	if(!Enemy) {
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
	}

	if (Enemy) {
		FVector Velocity{Enemy->GetVelocity()};
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
}
