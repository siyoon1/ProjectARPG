// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_MoveToAttackRange.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"

UC_BTTask_MoveToAttackRange::UC_BTTask_MoveToAttackRange()
{
	NodeName = TEXT("Move To Attack Range");
}

EBTNodeResult::Type UC_BTTask_MoveToAttackRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();

	if (!AICon)
		return EBTNodeResult::Failed;

	AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(AICon->GetPawn());

	if (!pEnemy)
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	AActor* pTarget = Cast<AActor>(BB->GetValueAsObject(AC_EnemyController::TargetActorKey));

	if (!pTarget)
		return EBTNodeResult::Failed;

	float fDesireRange = pEnemy->getAttackIdealRange();

	FVector vToEnemy = (pEnemy->GetActorLocation() - pTarget->GetActorLocation()).GetSafeNormal();

	FVector vDesiredLoc = pTarget->GetActorLocation() + vToEnemy * fDesireRange;

	AICon->MoveToLocation(vDesiredLoc, 30.f);

	return EBTNodeResult::Succeeded;
}
