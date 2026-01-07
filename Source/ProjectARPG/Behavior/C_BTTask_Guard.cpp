// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTTask_Guard.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UC_BTTask_Guard::UC_BTTask_Guard()
{
	NodeName = TEXT("Guard");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UC_BTTask_Guard::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	CachedOwnerComp = &OwnerComp;

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
        return EBTNodeResult::Failed;

    AC_EnemyCharacter* Enemy =
        Cast<AC_EnemyCharacter>(AICon->GetPawn());
    if (!Enemy)
        return EBTNodeResult::Failed;

    const FS_EnemyCombatProfile& Profile =
        Enemy->getCombatProfile();

    Enemy->m_onGuardFinished.RemoveAll(this);
    Enemy->m_onGuardFinished.AddUObject(
        this,
        &UC_BTTask_Guard::onGuardFinished
    );

    if (!Enemy->guardForDuration(Profile.fGuardDuration))
        return EBTNodeResult::Failed;

    return EBTNodeResult::InProgress;
}

void UC_BTTask_Guard::onGuardFinished()
{
    if (!CachedOwnerComp)
        return;

    UBlackboardComponent* BB =
        CachedOwnerComp->GetBlackboardComponent();

    if (BB)
    {
        BB->SetValueAsEnum(
            AC_EnemyController::AIActionKey,
            static_cast<uint8>(E_EnemyCombatAction::None)
        );
    }

    if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
    {
        if (AC_EnemyCharacter* Enemy =
            Cast<AC_EnemyCharacter>(AICon->GetPawn()))
        {
            Enemy->m_onGuardFinished.RemoveAll(this);
        }
    }

    FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
