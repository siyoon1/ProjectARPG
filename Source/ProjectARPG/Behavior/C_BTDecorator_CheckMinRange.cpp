// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_CheckMinRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

UC_BTDecorator_CheckMinRange::UC_BTDecorator_CheckMinRange()
{
	NodeName = "Dist < MinRange";
}

bool UC_BTDecorator_CheckMinRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
        return false;

    AC_EnemyCharacter* Enemy =
        Cast<AC_EnemyCharacter>(AICon->GetPawn());
    if (!Enemy)
        return false;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
        return false;

    const float Dist =
        BB->GetValueAsFloat(AC_EnemyController::DistKey);

    return !Enemy->canConsiderAttack(Dist);
}
