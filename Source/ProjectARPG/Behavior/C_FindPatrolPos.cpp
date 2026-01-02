// Fill out your copyright notice in the Description page of Project Settings.


#include "C_FindPatrolPos.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/AI/C_DetectComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


UC_FindPatrolPos::UC_FindPatrolPos()
{
	Interval = 0.2f;
}

void UC_FindPatrolPos::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* pEnemy = OwnerComp.GetAIOwner()->GetPawn();

	if (!pEnemy)
		return;

	UC_DetectComponent* pDetectCom = pEnemy->FindComponentByClass<UC_DetectComponent>();

	if (!pDetectCom)
		return;

	AActor* pTarget = pDetectCom->getDetectedTarget();

	
	OwnerComp.GetBlackboardComponent()->SetValueAsObject(AC_EnemyController::TargetActorKey, pTarget);

	if (OwnerComp.GetBlackboardComponent()->GetValueAsObject(AC_EnemyController::TargetActorKey))
	{
		AC_EnemyCharacter* Enemy = Cast<AC_EnemyCharacter>(pEnemy);
		if (Enemy)
			Enemy->setInCombat(true);
	}
	else
	{
		AC_EnemyCharacter* Enemy = Cast<AC_EnemyCharacter>(pEnemy);
		if (Enemy)
			Enemy->setInCombat(false);
	}
	

}
