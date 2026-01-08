// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTService_UpdateTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/AI/C_DetectComponent.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

UC_BTService_UpdateTarget::UC_BTService_UpdateTarget()
{
	Interval = 0.05f;
	bNotifyBecomeRelevant = true;
}

void UC_BTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();

	if (!AICon)
		return;

	APawn* pOwner = AICon->GetPawn();

	if (!pOwner)
		return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!BB)
		return;

	UC_DetectComponent* DetectComp = pOwner->FindComponentByClass<UC_DetectComponent>();

	if (!DetectComp)
		return;

	AActor* pTarget = DetectComp->getDetectedTarget();

	if (!pTarget)
	{
		BB->ClearValue(AC_EnemyController::TargetActorKey);
		BB->SetValueAsFloat(AC_EnemyController::DistKey, -1.f);
		return;
	}

	if (DetectComp->isDetecting())
	{
		BB->SetValueAsObject(AC_EnemyController::TargetActorKey, pTarget);
	}
	


	if (pTarget)
	{
		const float fDist = FVector::Dist(pOwner->GetActorLocation(), pTarget->GetActorLocation());

		BB->SetValueAsFloat(AC_EnemyController::DistKey, fDist);
	}
	else
	{
		BB->SetValueAsFloat(AC_EnemyController::DistKey, -1.f);
	}
}
