// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_HighestScroe.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"



UC_BTDecorator_HighestScroe::UC_BTDecorator_HighestScroe()
{
    NodeName = TEXT("Is Highest Score");
}

bool UC_BTDecorator_HighestScroe::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    UBlackboardComponent* BB =
        OwnerComp.GetBlackboardComponent();

    if (!BB)
        return false;

    const float MyScore =
        BB->GetValueAsFloat(MyScoreKey.SelectedKeyName);

    if (MyScore <= 0.f)
        return false;

    const float Attack =
        BB->GetValueAsFloat(AC_EnemyController::AttackScoreKey);

    const float Guard =
        BB->GetValueAsFloat(AC_EnemyController::GuardScoreKey);

    const float Repo =
        BB->GetValueAsFloat(AC_EnemyController::RepositionScoreKey);

    const float Chase =
        BB->GetValueAsFloat(AC_EnemyController::ChaseScorekey);

    const float MaxScore =
        FMath::Max(
            FMath::Max(Attack, Guard),
            FMath::Max(Repo, Chase));

    const float Threshold = 0.2f;

    return (MyScore >= MaxScore &&
        MyScore > Threshold);
}
