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
	if (!m_OwnerEnemy)
		return false;

	FName AttackRow;
	if (!decideNextAttack(fDist, AttackRow))
		return false;

	return executeAttack(AttackRow);
}

void UC_EnemyAttackComponent::getAttackCandidates(float fDist, TArray<FName>& OutCandidates) const
{
	OutCandidates.Empty();

	if (!m_AttackDataTable)
		return;

	for (const auto& Row : m_AttackDataTable->GetRowMap())
	{
		const FS_AttackData* Data =
			m_AttackDataTable->FindRow<FS_AttackData>(Row.Key, TEXT("EnemyAttack"));

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
			m_AttackDataTable->FindRow<FS_AttackData>(Row, TEXT("WeightCalc"));

		float Weight = Data->AI.BaseWeight;

		// 거리 선호 보정
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
	if (!m_AttackDataTable || !m_OwnerEnemy)
		return false;

	const FS_AttackData* Data =
		m_AttackDataTable->FindRow<FS_AttackData>(Row, TEXT("Execute"));

	if (!Data)
		return false;

	m_OwnerEnemy->markAttackUsed(Row, Data->AI.MinReuseTime);

	return m_OwnerEnemy->playAttack(Data);
}

