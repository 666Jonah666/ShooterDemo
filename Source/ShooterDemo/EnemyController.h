// Andrei Nikitin 2022

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERDEMO_API AEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:

	//blackboard component for this enemy
	UPROPERTY(BlueprintReadWrite, Category = "AI Behaviour", meta = (AllowPrivateAccess = "true"))
	UBlackboardComponent* BlackboardComponent;

	//Behaviour Tree component for this enemy
	UPROPERTY(BlueprintReadWrite, Category = "AI Behaviour", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

public:
	FORCEINLINE UBlackboardComponent* GetEnemyBlackboardComponent() const { return  BlackboardComponent; }
};
