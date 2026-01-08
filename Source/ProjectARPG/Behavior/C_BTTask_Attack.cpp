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
    CachedOwnerComp = &OwnerComp;

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
        return EBTNodeResult::Failed;

    AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(AICon->GetPawn());
    if (!pEnemy)
        return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
        return EBTNodeResult::Failed;


    const FName AttackRow =
        BB->GetValueAsName(AC_EnemyController::SelectAttackKey);

    const FS_AttackData* pData = pEnemy->getAttackData(AttackRow);
    if (!pData)
        return EBTNodeResult::Failed;

    // 안전: 기존 바인딩 제거
    pEnemy->m_onAttackFinished.RemoveAll(this);
    pEnemy->m_onAttackFinished.AddUObject(
        this,
        &UC_BTTask_Attack::onAttackEnded
    );

    bool bStarted = pEnemy->attack(pData);
    if (!bStarted)
    {
        BB->SetValueAsBool(AC_EnemyController::IntentLockedKey, false);
        return EBTNodeResult::Failed;
    }

    BB->SetValueAsBool(
        AC_EnemyController::IntentLockedKey,
        true);

    return EBTNodeResult::InProgress;
}

void UC_BTTask_Attack::onAttackEnded()
{
	if (!CachedOwnerComp)
		return;

    UBlackboardComponent* BB =
        CachedOwnerComp->GetBlackboardComponent();


    if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
    {
        if (AC_EnemyCharacter* Enemy =
            Cast<AC_EnemyCharacter>(AICon->GetPawn()))
        {
            Enemy->m_onAttackFinished.RemoveAll(this);
        }
    }

    BB->SetValueAsBool(
        AC_EnemyController::IntentLockedKey,
        false);

	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
