// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_Attack.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/AI/C_AIAttackComponent.h"
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

    AC_EnemyCharacter* Enemy =
        Cast<AC_EnemyCharacter>(AICon->GetPawn());
    if (!Enemy)
        return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
        return EBTNodeResult::Failed;

    UC_AIAttackComponent* EnemyAttackComp =
        Enemy->getEnemyAttackComponent();
    if (!EnemyAttackComp)
        return EBTNodeResult::Failed;

    const float Dist =
        BB->GetValueAsFloat(AC_EnemyController::DistKey);

    // 공격 종료 콜백 바인딩
    Enemy->m_onAttackFinished.RemoveAll(this);
    Enemy->m_onAttackFinished.AddUObject(
        this,
        &UC_BTTask_Attack::onAttackEnded
    );

    // 공격 시도
    if (!EnemyAttackComp->tryExecuteAttack(Dist))
    {
        BB->SetValueAsBool(
            AC_EnemyController::IntentLockedKey,
            false);
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
