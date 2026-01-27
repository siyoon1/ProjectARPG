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

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;

	AC_EnemyCharacter* Enemy =
		Cast<AC_EnemyCharacter>(AICon->GetPawn());
	if (!Enemy)
		return EBTNodeResult::Failed;

	// 이미 Guard 중이면 그대로 유지
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
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return;

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return;

	AC_EnemyCharacter* Enemy =
		Cast<AC_EnemyCharacter>(AICon->GetPawn());
	if (!Enemy)
		return;

	// Intent가 Guard가 아니면 즉시 종료
	const E_CombatIntent CurrentIntent =
		(E_CombatIntent)BB->GetValueAsEnum(
			AC_EnemyController::IntentKey);

	if (CurrentIntent != E_CombatIntent::Guard)
	{
		if (Enemy->isGuard())
		{
			Enemy->endGuard();
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Enemy 내부 판단으로 Guard 해제
	if (!Enemy->canReleaseGuard())
		return;

	Enemy->endGuard();

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

