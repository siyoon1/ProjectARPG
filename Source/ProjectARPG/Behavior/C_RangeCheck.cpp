// Fill out your copyright notice in the Description page of Project Settings.


#include "C_RangeCheck.h"
#include "C_RangeCheck.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/Interface/C_CombatInterface.h"

bool UC_RangeCheck::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* pBB = OwnerComp.GetBlackboardComponent();
	if (!pBB)
		return false;

	APawn* pOwner = OwnerComp.GetAIOwner()->GetPawn();
	if (!pOwner)
		return false;

	const AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pOwner);

	AActor* pTarget = Cast<AActor>(pBB->GetValueAsObject(AC_EnemyController::TargetActorKey));
	if (!pTarget)
		return false;

	float fDist = FVector::Dist(pEnemy->GetActorLocation(), pTarget->GetActorLocation());

	float fMin = pEnemy->getAttackMinRange();
	float fMax = pEnemy->getAttackMaxRange();

	return fDist >= fMin && fDist <= fMax;
}
