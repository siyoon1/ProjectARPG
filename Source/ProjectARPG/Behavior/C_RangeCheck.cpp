// Fill out your copyright notice in the Description page of Project Settings.


#include "C_RangeCheck.h"
#include "C_RangeCheck.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Interface/C_CombatInterface.h"

bool UC_RangeCheck::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	OwnerComp.GetBlackboardComponent()->SetValueAsFloat(AC_EnemyController::DistKey, 150.f);

	const UBlackboardComponent* pBB = OwnerComp.GetBlackboardComponent();
	if (!pBB)
		return false;

	const APawn* pOwner = OwnerComp.GetAIOwner()->GetPawn();
	if (!pOwner)
		return false;

	float fAttackDist = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(AC_EnemyController::DistKey);


	AActor* pTarget = Cast<AActor>(pBB->GetValueAsObject(AC_EnemyController::TargetActorKey));
	if (!pTarget)
		return false;

	

	float fDist = FVector::Dist(pOwner->GetActorLocation(), pTarget->GetActorLocation());

	return fDist <= fAttackDist;
}
