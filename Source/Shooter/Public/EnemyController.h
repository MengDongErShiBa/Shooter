// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()
public:
	AEnemyController();

	// 成功控制角色时调用
	virtual void OnPossess(APawn* InPawn) override;
	
private:
	// 敌人黑板组件
	UPROPERTY(BlueprintReadWrite, Category = AIBehavior, meta = (AllowPrivateAccess = "true"))
	class UBlackboardComponent* BlackboardComponent;

	// 敌人的行为树
	UPROPERTY(BlueprintReadWrite, Category = AIBehavior, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

public:
	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorTreeComponent() const { return BehaviorTreeComponent; }
};
