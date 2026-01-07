// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_CheckAttackRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

UC_BTDecorator_CheckAttackRange::UC_BTDecorator_CheckAttackRange()
{
	NodeName = "Check Attack Range";
}

bool UC_BTDecorator_CheckAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return false;

	AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(AICon->GetPawn());

	if (!pEnemy)
		return false;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!BB)
		return false;

	FName SelectedAttackRow = BB->GetValueAsName(AC_EnemyController::SelectAttackKey);
	const FS_AttackData* pData = pEnemy->getAttackData(SelectedAttackRow);

	if (!pData)
		return false;

	AActor* pTarget = Cast<AActor>(BB->GetValueAsObject(AC_EnemyController::TargetActorKey));
	if (!pTarget)
		return false;

	const float fDist = BB->GetValueAsFloat(AC_EnemyController::DistKey);

	if (pEnemy->isExecutingAction())
		return true;

	return fDist >= pData->fMinRange && fDist <= pData->fRange;
}
