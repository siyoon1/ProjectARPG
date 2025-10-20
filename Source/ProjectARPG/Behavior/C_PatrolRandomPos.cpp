// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PatrolRandomPos.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

EBTNodeResult::Type UC_PatrolRandomPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type res = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* pEnemy = OwnerComp.GetAIOwner()->GetPawn();
	if (!pEnemy)
		return EBTNodeResult::Failed;

	UNavigationSystemV1* pNav = UNavigationSystemV1::GetNavigationSystem(pEnemy->GetWorld());
	if (!pNav)
		return EBTNodeResult::Failed;

	FVector vStartPos = OwnerComp.GetBlackboardComponent()->GetValueAsVector(AC_EnemyController::StartPosKey);
	FNavLocation vPos{};

	if (pNav->GetRandomPointInNavigableRadius(vStartPos, 500.f, vPos))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(AC_EnemyController::PatrolPosKey, vPos.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
