// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName AC_EnemyController::TargetActorKey = TEXT("TargetActor");
const FName AC_EnemyController::StartPosKey = TEXT("StartPos");
const FName AC_EnemyController::PatrolPosKey = TEXT("PatrolPos");

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
	}
}
