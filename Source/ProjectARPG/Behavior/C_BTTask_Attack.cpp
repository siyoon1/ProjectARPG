// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_Attack.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

UC_BTTask_Attack::UC_BTTask_Attack()
{
	NodeName = TEXT("Attack");
}

EBTNodeResult::Type UC_BTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;

	AC_EnemyCharacter* pEnemy =
		Cast<AC_EnemyCharacter>(AICon->GetPawn());

	if (!pEnemy)
		return EBTNodeResult::Failed;

	// 이미 다른 행동 중이면 실패
	if (pEnemy->isExecutingAction())
		return EBTNodeResult::Failed;

	pEnemy->attack();

	return EBTNodeResult::Succeeded;
}
