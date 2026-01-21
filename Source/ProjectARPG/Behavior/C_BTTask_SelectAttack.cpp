// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_SelectAttack.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/ActorComponents/C_EnemyAttackComponent.h"

UC_BTTask_SelectAttack::UC_BTTask_SelectAttack()
{
	NodeName = "SelectAttack";
}

EBTNodeResult::Type UC_BTTask_SelectAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();

	if (!AICon)
		return EBTNodeResult::Failed;

	AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(AICon->GetPawn());

	if (!pEnemy)
		return EBTNodeResult::Failed;

	if (!pEnemy->canDecideAction())
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail: isExecutingAction"));
		return EBTNodeResult::Failed;
	}
		

	if (GetWorld()->GetTimeSeconds() < pEnemy->getNextActionTime())
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail: Cooldown"));
		return EBTNodeResult::Failed;
	}
		

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	const float fDist =
		BB->GetValueAsFloat(AC_EnemyController::DistKey);

	FName SelectedRow = NAME_None;

	if (!pEnemy->getAttackComponent()->tryExecuteAttack(fDist))
		return EBTNodeResult::Failed;

	BB->SetValueAsName(AC_EnemyController::SelectAttackKey, SelectedRow);

	return EBTNodeResult::Succeeded;
}
