// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"

const FName AC_EnemyController::TargetActorKey = TEXT("TargetActor");
const FName AC_EnemyController::StartPosKey = TEXT("StartPos");
const FName AC_EnemyController::PatrolPosKey = TEXT("PatrolPos");
const FName AC_EnemyController::DistKey = TEXT("DistanceToTarget");
const FName AC_EnemyController::AttackMoveLocationKey = TEXT("AttackMovePos");
const FName AC_EnemyController::AttackScoreKey = TEXT("AttackScore");
const FName AC_EnemyController::GuardScoreKey = TEXT("GuardScore");
const FName AC_EnemyController::RepositionScoreKey = TEXT("RepositionScore");
const FName AC_EnemyController::ChaseScorekey = TEXT("ChaseScore");

AC_EnemyController::AC_EnemyController()
{
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BT(TEXT("/Game/AI/bt-Enemy.bt-Enemy"));
	if (BT.Succeeded())
	{
		m_BT = BT.Object;
	}

	static ConstructorHelpers::FObjectFinder<UBlackboardData> BB(TEXT("/Game/AI/bb-Enemy.bb-Enemy"));
	if (BB.Succeeded())
	{
		m_BB = BB.Object;
	}
}

void AC_EnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UBlackboardComponent* pBBcom = Blackboard;

	if (UseBlackboard(m_BB, pBBcom))
	{
		RunBehaviorTree(m_BT);

		if (InPawn)
		{
			Blackboard->SetValueAsVector(StartPosKey, InPawn->GetActorLocation());
		}
	}

	
}

void AC_EnemyController::executionFinishied(AC_CombatCharacter* pTarget)
{
	StopMovement();

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->ClearValue(AC_EnemyController::PatrolPosKey);
		BB->ClearValue(AC_EnemyController::AttackMoveLocationKey);


		BB->SetValueAsObject(AC_EnemyController::TargetActorKey, pTarget);

	}
	if (BrainComponent)
	{
		BrainComponent->RestartLogic();
	}
}
