// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_Attack.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UC_BTTask_Attack::UC_BTTask_Attack()
{
	NodeName = TEXT("Attack");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UC_BTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;

	AC_EnemyCharacter* pEnemy =
		Cast<AC_EnemyCharacter>(AICon->GetPawn());

	if (!pEnemy)
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	CachedOwnerComp = &OwnerComp;

	FName Row = BB->GetValueAsName(AC_EnemyController::SelectAttackKey);

	const FS_AttackData* pData = pEnemy->getAttackData(Row);

	if (!pData)
		return EBTNodeResult::Failed;

	pEnemy->m_onAttackFinished.AddUObject(this, &ThisClass::onAttackEnded);
	pEnemy->attack(pData);

	return EBTNodeResult::InProgress;
}

void UC_BTTask_Attack::onAttackEnded()
{
	if (!CachedOwnerComp)
		return;

	if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
	{
		if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(AICon->GetPawn()))
		{
			pEnemy->m_onAttackFinished.RemoveAll(this);
		}
	}

	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
