// Fill out your copyright notice in the Description page of Project Settings.


#include "C_FindPatrolPos.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "DrawDebugHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/OverlapResult.h"

void UC_FindPatrolPos::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* pEnemy = OwnerComp.GetAIOwner()->GetPawn();

	if (!pEnemy)
		return;

	FVector vCenter = pEnemy->GetActorLocation();
	float fDetectRadius = 600.f;

	TArray<FOverlapResult> listOverlap{};

	FCollisionQueryParams Params(NAME_None, false, pEnemy);

	bool bHasHit = 
	pEnemy->GetWorld()->OverlapMultiByChannel(listOverlap, vCenter, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(fDetectRadius), Params);

//#ifdef DEBUG_DRAW
	//DrawDebugSphere(pEnemy->GetWorld(), vCenter, fDetectRadius, 20, FColor::Red, false, 1.f);
//#endif // DEBUG_DRAW

	if (bHasHit)
	{
		ACharacter* pTarget = nullptr;

		for (const FOverlapResult& Object : listOverlap)
		{
			AActor* pAct = Object.GetActor();
			if (ACharacter* pChar = Cast<ACharacter>(pAct))
			{
				pTarget = pChar;
			}

		}
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(AC_EnemyController::TargetActorKey, pTarget);
	}

}
