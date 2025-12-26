// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyCombatAction.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

EBTNodeResult::Type UC_EnemyCombatAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type res =  Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* pPawn = OwnerComp.GetAIOwner()->GetPawn();

	if (!pPawn)
		return EBTNodeResult::Failed;

	AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pPawn);
	
	if (!pEnemy)
		return EBTNodeResult::Failed;


	pEnemy->setInCombat(true);
	return EBTNodeResult::Succeeded;
	
}
