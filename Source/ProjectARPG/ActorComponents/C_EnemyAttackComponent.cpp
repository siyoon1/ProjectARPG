// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyAttackComponent.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

// Sets default values for this component's properties
UC_EnemyAttackComponent::UC_EnemyAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UC_EnemyAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	m_OwnerEnemy = Cast<AC_EnemyCharacter>(GetOwner());
}


// Called every frame
void UC_EnemyAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UC_EnemyAttackComponent::tryExecuteAttack(float fDist)
{
	UE_LOG(LogTemp, Warning, TEXT("[EnemyAttack] tryExecuteAttack Dist=%.1f"), fDist);


	if (!m_OwnerEnemy)
		return false;

	FName AttackRow;
	if (!decideNextAttack(fDist, AttackRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAttack] decideNextAttack FAILED"));
		return false;
	}


	UE_LOG(LogTemp, Warning, TEXT("[EnemyAttack] Selected AttackRow = %s"), *AttackRow.ToString());
	return executeAttack(AttackRow);
}

void UC_EnemyAttackComponent::getAttackCandidates(float fDist, TArray<FName>& OutCandidates) const
{
	UE_LOG(LogTemp, Warning, TEXT("[EnemyAttack] getAttackCandidates Dist=%.1f"), fDist);

	OutCandidates.Empty();

	const UDataTable* DT = m_OwnerEnemy->getAttackDataTable();

	if (!DT)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[EnemyAttack] Owner AttackDataTable NULL"));
		return;
	}

	for (const auto& Row : DT->GetRowMap())
	{
		const FS_AttackData* Data =
			m_OwnerEnemy->getAttackData(Row.Key);

		if (!Data)
			continue;

		if (!canUseAttack(Row.Key))
			continue;

		if (!isAttackInRange(*Data, fDist))
			continue;

		OutCandidates.Add(Row.Key);
	}
}

bool UC_EnemyAttackComponent::decideNextAttack(float fDist, FName& OutAttackRow) const
{
	TArray<FName> Candidates;
	getAttackCandidates(fDist, Candidates);

	if (Candidates.Num() == 0)
		return false;

	float TotalWeight = 0.f;
	TArray<float> Weights;

	for (FName Row : Candidates)
	{
		const FS_AttackData* Data =
			m_OwnerEnemy->getAttackData(Row);

		if (!Data)
			continue;

		float Weight = Data->AI.BaseWeight;

		const float DistFactor =
			1.f - FMath::Abs(fDist - Data->AI.IdealRange) / Data->AI.IdealRange;

		Weight *= FMath::Clamp(DistFactor, 0.2f, 1.f);

		Weights.Add(Weight);
		TotalWeight += Weight;
	}

	const float Pick = FMath::FRandRange(0.f, TotalWeight);
	float Acc = 0.f;

	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		Acc += Weights[i];
		if (Pick <= Acc)
		{
			OutAttackRow = Candidates[i];
			return true;
		}
	}

	return false;
}

bool UC_EnemyAttackComponent::canUseAttack(FName Row) const
{
	if (!m_OwnerEnemy)
		return false;

	const FS_AttackRuntimeState* State =
		m_OwnerEnemy->getAttackRuntimeState(Row);

	if (!State)
		return true;

	const float Now = GetWorld()->GetTimeSeconds();
	return (Now - State->LastUsedTime) >= State->Cooldown;
}

bool UC_EnemyAttackComponent::isAttackInRange(const FS_AttackData& Data, float fDist) const
{
	return fDist >= Data.AI.MinRange &&
		fDist <= Data.AI.MaxRange;
}

bool UC_EnemyAttackComponent::executeAttack(FName Row)
{
	if (!m_OwnerEnemy)
		return false;

	m_OwnerEnemy->markAttackUsed(Row,
		m_OwnerEnemy->getAttackData(Row)->AI.MinReuseTime);

	return m_OwnerEnemy->playAttackByRow(Row);


}

