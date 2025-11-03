// Fill out your copyright notice in the Description page of Project Settings.


#include "C_LookAt.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UC_LookAt::ExecuteTask(UBehaviorTreeComponent& OwnerCom, uint8* NodeMemory)
{
	EBTNodeResult::Type res =  Super::ExecuteTask(OwnerCom, NodeMemory);


	APawn* pPawn = OwnerCom.GetAIOwner()->GetPawn();

	if (!pPawn)
		return EBTNodeResult::Failed;

	AActor* pTarget = Cast<AActor>(OwnerCom.GetBlackboardComponent()->GetValueAsObject(AC_EnemyController::TargetActorKey));

	if (!pTarget)
		return EBTNodeResult::Failed;

	FVector vTargetDir = pTarget->GetActorLocation() - pPawn->GetActorLocation();
	vTargetDir.Z = 0.f;

	if (vTargetDir.IsNearlyZero())
		return EBTNodeResult::Succeeded;

	FRotator rLookRot = vTargetDir.Rotation();


	FRotator rNewRot = FMath::RInterpTo(pPawn->GetActorRotation(), rLookRot, GetWorld()->GetDeltaSeconds(), 10.f);

	pPawn->SetActorRotation(rNewRot);

	return EBTNodeResult::Succeeded;
}
