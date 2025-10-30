// Fill out your copyright notice in the Description page of Project Settings.


#include "C_RangeCheck.h"
#include "C_RangeCheck.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Interface/C_CombatInterface.h"

bool UC_RangeCheck::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool res = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	OwnerComp.GetBlackboardComponent()->SetValueAsFloat(AC_EnemyController::DistKey, 200.f);
	float fAttackDist = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(AC_EnemyController::DistKey);

	APawn* pOwner = OwnerComp.GetAIOwner()->GetPawn();
	if (!pOwner)
		return false;

	APawn* pTarget = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AC_EnemyController::TargetActorKey));
	if (!pTarget)
		return false;

	if (pTarget->GetClass()->ImplementsInterface(UC_CombatInterface::StaticClass()))
	{
		float fDist = FVector::Dist(pOwner->GetActorLocation(), IC_CombatInterface::Execute_getLocation(pTarget));
		return fDist <= fAttackDist;
	}



	return false;
}
