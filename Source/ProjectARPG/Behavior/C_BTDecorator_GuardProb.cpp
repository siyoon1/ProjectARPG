// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_GuardProb.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"

UC_BTDecorator_GuardProb::UC_BTDecorator_GuardProb()
{
	NodeName = TEXT("Random <= GuardProb");
}

bool UC_BTDecorator_GuardProb::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return false;

	const float fGuardProb = BB->GetValueAsFloat(
		AC_EnemyController::GuardProbKey
	);

	if (fGuardProb <= 0.f)
		return false;

	if (fGuardProb >= 1.f)
		return true;

	return FMath::FRand() <= fGuardProb;
}
