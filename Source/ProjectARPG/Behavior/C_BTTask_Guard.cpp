// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_Guard.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

UC_BTTask_Guard::UC_BTTask_Guard()
{
	NodeName = TEXT("Guard");
}

EBTNodeResult::Type UC_BTTask_Guard::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;

	AC_EnemyCharacter* pEnemy =
		Cast<AC_EnemyCharacter>(AICon->GetPawn());

	if (!pEnemy)
		return EBTNodeResult::Failed;

	if (pEnemy->isExecutingAction())
		return EBTNodeResult::Failed;

	pEnemy->guardForDuration(0.6f);

	return EBTNodeResult::Succeeded;
}
