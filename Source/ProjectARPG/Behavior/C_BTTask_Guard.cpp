// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_Guard.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UC_BTTask_Guard::UC_BTTask_Guard()
{
	NodeName = TEXT("Guard");
	bCreateNodeInstance = true;
    bNotifyTick = true;
}

EBTNodeResult::Type UC_BTTask_Guard::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AC_EnemyCharacter* Enemy =
		Cast<AC_EnemyCharacter>(AICon->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	if (!Enemy->isGuard())
	{
		if (!Enemy->startGuard())
			return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

void UC_BTTask_Guard::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	AC_EnemyCharacter* Enemy =
		Cast<AC_EnemyCharacter>(AICon->GetPawn());
	if (!Enemy) return;

	if (Enemy->canReleaseGuard())
	{
		Enemy->endGuard();
		FinishLatentTask(
			OwnerComp,
			EBTNodeResult::Succeeded);
	}
}

