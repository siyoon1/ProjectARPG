// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "C_EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API AC_EnemyController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TObjectPtr<class UBehaviorTree> m_BT;
	UPROPERTY()
	TObjectPtr<class UBlackboardData> m_BB;

public:
	AC_EnemyController();

	virtual void OnPossess(APawn* InPawn) override;

	static const FName TargetActorKey;
	static const FName StartPosKey;
	static const FName PatrolPosKey;
	static const FName DistKey;
	static const FName IsCombatKey;
	static const FName AttackProbKey;
	static const FName GuardProbKey;;
	
};
