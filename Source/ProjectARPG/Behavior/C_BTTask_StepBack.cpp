// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_StepBack.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/Animation/C_EnemyAnim.h"
#include "ProjectARPG/AI/C_AIAttackComponent.h"

UC_BTTask_StepBack::UC_BTTask_StepBack()
{
	NodeName = "StepBack";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UC_BTTask_StepBack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

    CachedOwnerComp = &OwnerComp;

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

    // 중복 바인딩 방지
    Enemy->m_onStepBackFinished.RemoveAll(this);
    Enemy->m_onStepBackFinished.AddUObject(
        this,
        &UC_BTTask_StepBack::onStepBackFinished
    );

    if (!Enemy->playStepBack())
    {
        Enemy->m_onStepBackFinished.RemoveAll(this);
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::InProgress;
}

void UC_BTTask_StepBack::onStepBackFinished()
{
    if (!CachedOwnerComp)
        return;

    if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
    {
        if (AC_EnemyCharacter* Enemy =
            Cast<AC_EnemyCharacter>(AICon->GetPawn()))
        {
            Enemy->m_onStepBackFinished.RemoveAll(this);
        }
    }

    FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
