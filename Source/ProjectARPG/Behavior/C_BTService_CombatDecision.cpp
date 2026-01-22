// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTService_CombatDecision.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/ActorComponents/C_EnemyAttackComponent.h"

void UC_BTService_CombatDecision::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();

	if (!AICon)
		return;

	AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(AICon->GetPawn());
	if (!pEnemy)
		return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!BB)
		return;

	if (BB->GetValueAsBool(AC_EnemyController::IntentLockedKey))
		return;

	if (!pEnemy->canDecideAction())
		return;

	AActor* Target =
		Cast<AActor>(BB->GetValueAsObject("TargetActor"));

	if (BB->GetValueAsBool(AC_EnemyController::IntentLockedKey))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CombatDecision] IntentLocked = true, skip decide"));
		return;
	}

	if (!Target)
	{
		BB->SetValueAsEnum(
			AC_EnemyController::IntentKey,
			(uint8)E_CombatIntent::None);
		return;
	}

	const float Dist = BB->GetValueAsFloat(AC_EnemyController::DistKey);


	const FS_EnemyCombatProfile& Profile = pEnemy->getCombatProfile();

	if (Dist > Profile.fPreferredRange * 1.3f)
	{
		BB->SetValueAsEnum(
			AC_EnemyController::IntentKey,
			(uint8)E_CombatIntent::Chase);
		return;
	}


	float AttackW = Profile.fAttackProbability;
	float GuardW = Profile.fGuardProbability;
	float RepoW = 0.3f;

	// 거리 기반 보정
	if (Dist < Profile.fPreferredRange * 0.8f)
	{
		AttackW *= 1.2f;
		GuardW *= 0.8f;
	}
	else if (Dist > Profile.fPreferredRange * 1.1f)
	{
		AttackW *= 0.7f;
		RepoW *= 1.3f;
	}

	const E_CombatIntent LastIntent =
		(E_CombatIntent)BB->GetValueAsEnum(AC_EnemyController::LastIntentKey);

	if (LastIntent == E_CombatIntent::Guard)
	{
		AttackW = 1.3f;
		GuardW *= 0.2f;
		RepoW *= 1.2f;
	}

	const float Sum = AttackW + GuardW + RepoW;
	const float Pick = FMath::FRandRange(0.f, Sum);

	E_CombatIntent Intent;

	

	if (Pick < AttackW)
		Intent = E_CombatIntent::Attack;
	else if (Pick < AttackW + GuardW)
		Intent = E_CombatIntent::Guard;
	else
		Intent = E_CombatIntent::Reposition;

	UE_LOG(LogTemp, Warning,
		TEXT("[CombatDecision] Dist=%.1f A=%.2f G=%.2f R=%.2f Pick=%.2f -> Intent=%d"),
		Dist, AttackW, GuardW, RepoW, Pick, (int32)Intent);


	BB->SetValueAsEnum(
		AC_EnemyController::IntentKey,
		(uint8)Intent);

	BB->SetValueAsEnum(
		AC_EnemyController::LastIntentKey,
		(uint8)Intent);
	
}
