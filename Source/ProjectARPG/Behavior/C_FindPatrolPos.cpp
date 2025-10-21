// Fill out your copyright notice in the Description page of Project Settings.


#include "C_FindPatrolPos.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
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

		AActor* pTargetActor = Cast<AActor>(pTarget);
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(AC_EnemyController::TargetActorKey, pTargetActor);

		if (OwnerComp.GetBlackboardComponent()->GetValueAsObject(AC_EnemyController::TargetActorKey))
		{
			AC_EnemyCharacter* Enemy = Cast<AC_EnemyCharacter>(pEnemy);
			if (Enemy)
				Enemy->showHpBar(true);
		}
		else
		{
			AC_EnemyCharacter* Enemy = Cast<AC_EnemyCharacter>(pEnemy);
			if (Enemy)
				Enemy->showHpBar(false);
		}
	}

}
