// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"

#include "Enemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyController::AEnemyController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	check(BlackboardComponent);

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	check(BehaviorTreeComponent);
}

void AEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (AEnemy* Enemy = Cast<AEnemy>(InPawn))
	{
		if (auto Tree = Enemy->GetBehaviorTree())
		{
			BlackboardComponent->InitializeBlackboard(*Tree->GetBlackboardAsset());
		}
	}
}
