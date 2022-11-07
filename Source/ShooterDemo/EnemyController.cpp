// Andrei Nikitin 2022


#include "EnemyController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy.h"
#include "BehaviorTree/BehaviorTree.h"

AEnemyController::AEnemyController() {
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard Component"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("Behaviour Tree Component");

	//if false causes an error
	check(BlackboardComponent);
	check(BehaviorTreeComponent);
	
}

void AEnemyController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);

	if (!InPawn) {
		return;
	}

	AEnemy* Enemy = Cast<AEnemy>(InPawn);
	if (Enemy) {
		if (Enemy->GetBehaviourTree()) {
			BlackboardComponent->InitializeBlackboard(*(Enemy->GetBehaviourTree()->BlackboardAsset));
		}
	}
	
}
