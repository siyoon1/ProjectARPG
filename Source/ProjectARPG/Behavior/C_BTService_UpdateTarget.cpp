// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTService_UpdateTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/AI/C_DetectComponent.h"

UC_BTService_UpdateTarget::UC_BTService_UpdateTarget()
{
	Interval = 0.2f;
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

	UC_DetectComponent* DetectComp = pOwner->FindComponentByClass<UC_DetectComponent>();

	if (!DetectComp)
		return;

	AActor* pTarget = DetectComp->getDetectedTarget();

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!BB)
		return;

	BB->SetValueAsObject(AC_EnemyController::TargetActorKey, pTarget);
	BB->SetValueAsBool(AC_EnemyController::IsCombatKey, pTarget != nullptr);

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
