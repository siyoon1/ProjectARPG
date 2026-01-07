// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_StepBack.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/Animation/C_EnemyAnim.h"

UC_BTTask_StepBack::UC_BTTask_StepBack()
{
	NodeName = "StepBack";
}

EBTNodeResult::Type UC_BTTask_StepBack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;

	AC_EnemyCharacter* pEnemy =
		Cast<AC_EnemyCharacter>(AICon->GetPawn());

	if (!pEnemy)
		return EBTNodeResult::Failed;

	pEnemy->playStepBack();

	return EBTNodeResult::Succeeded;
}
