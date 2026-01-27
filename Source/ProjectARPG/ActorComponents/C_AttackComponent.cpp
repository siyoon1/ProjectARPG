// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AttackComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "ProjectARPG/ActorComponents/C_CombatStatComponent.h"


// Sets default values for this component's properties
UC_AttackComponent::UC_AttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_AttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	m_Owner = Cast<AC_CombatCharacter>(GetOwner());

}


// Called every frame
void UC_AttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_AttackComponent::startAttack(const FS_AttackData& AttackData)
{
	UE_LOG(LogTemp, Warning, TEXT("[AttackComponent] startAttack"));
	m_CurrentAttackData = &AttackData;
	m_HitActors.Empty();
}

void UC_AttackComponent::endAttack()
{
	m_CurrentAttackData = nullptr;
	m_bTracing = false;
	m_HitActors.Empty();


}

void UC_AttackComponent::startTrace()
{
	if (!m_CurrentAttackData)
	{
		UE_LOG(LogTemp, Error, TEXT("[AttackComponent] startTrace called but AttackData is NULL"));
		return;
	}
		
	UE_LOG(LogTemp, Warning, TEXT("[AttackComponent] Trace START"));
	m_bTracing = true;
	m_HitActors.Empty();

	m_PrevDir = FVector::ZeroVector;
}

void UC_AttackComponent::stopTrace()
{
	m_bTracing = false;
}

void UC_AttackComponent::tickTrace()
{
	if (!m_bTracing || !m_Owner || !m_CurrentAttackData)
		return;

	switch (m_CurrentAttackData->Combat.Property)
	{
	case E_AttackProperty::Thrust:
		tickThrustTrace();
		break;

	default:
		tickArcTrace();
		break;

	}
}

void UC_AttackComponent::applyHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!m_Owner || !m_CurrentAttackData)
		return;

	if (!HitActor->GetClass()->ImplementsInterface(UC_CombatInterface::StaticClass()))
		return;

	AC_CombatCharacter* Target = Cast<AC_CombatCharacter>(HitActor);
	if (Target && Target->isInvincibleAgainst(m_Owner))
		return;

	if (Target->getParryComponent())
	{
		FS_ParryResult ParryResult =
			Target->getParryComponent()->evaluateParry(*m_CurrentAttackData, m_Owner);

		if (ParryResult.Result == E_ParryResult::Parried)
		{
			m_bPostureBrokenByParry = true;

			const FVector ParryPoint = Hit.ImpactPoint;

			m_OnAttackParried.Broadcast(
				m_Owner,
				Target,
				ParryPoint
			);

			UE_LOG(LogTemp, Warning, TEXT("[AttackComponent] Parried by %s"),
				*Target->GetName());

			// 공격자 반응
			IC_ParryReaction::Execute_onParried(m_Owner, Target);

			// 방어자 성공 반응
			IC_ParryReaction::Execute_onParrySuccess(Target, m_Owner, ParryResult.Direction);

			// 공격자 체간 피해
			if (AC_CombatCharacter* AttackerChar = m_Owner)
			{
				AttackerChar->getStatComp()->applyPostureDamage(ParryResult.PostureDamageToAttacker,
					E_PostureBreakCause::Parry, Target);
			}

			return;
		}
	}

	const bool bGuarded =
		Target->isGuard() &&
		Target->isGuardingFront(m_Owner);

	E_HitResult HitResult = bGuarded
		? E_HitResult::Guarded
		: E_HitResult::Normal;

	IC_CombatInterface::Execute_takeDamage
	(
		HitActor,
		m_CurrentAttackData->Combat.Damage,
		m_CurrentAttackData->Combat.PostureDamage,
		HitResult,
		m_Owner
	);

	Target->onHitConfirmed(HitResult, m_Owner);
}

void UC_AttackComponent::sweepAttack(const FVector& Start, const FVector& End)
{
	float TraceRadius = 12.f;
	ECollisionChannel TraceChannel = ECC_GameTraceChannel3;

	TArray<FHitResult> HitResults{};
	FCollisionQueryParams Params{};
	Params.AddIgnoredActor(m_Owner);

	bool bHit =
		GetWorld()->SweepMultiByChannel
		(
			HitResults,
			Start,
			End,
			FQuat::Identity,
			TraceChannel,
			FCollisionShape::MakeSphere(TraceRadius),
			Params
		);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();

			if (!HitActor)
				continue;

			if (m_HitActors.Contains(HitActor))
				continue;

			m_HitActors.Add(HitActor);
			applyHit(HitActor, Hit);

			DrawDebugSphere(
				GetWorld(),
				Hit.ImpactPoint,
				TraceRadius,
				12,
				FColor::Red,
				false,
				0.05f
			);
		}


	}	

	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		FColor::Red,
		false,
		0.05f,
		0,
		2.f
	);


}

void UC_AttackComponent::tickArcTrace()
{
	FVector CurStart = m_Owner->getTraceStartLocation();
	FVector CurEnd = m_Owner->getTraceEndLocation();
	FVector CurDir = (CurEnd - CurStart).GetSafeNormal();
	float BladeLength = FVector::Distance(CurStart, CurEnd);

	if (m_PrevDir.IsNearlyZero())
	{
		m_PrevDir = CurDir;
	}

	// 이전 -> 현재 각도 계산
	const float Angle = FMath::Acos(FVector::DotProduct(m_PrevDir, CurDir));

	// 분할 개수 결정 5도 단위로 Sweep
	const int32 Steps = FMath::Clamp(FMath::CeilToInt(FMath::RadiansToDegrees(Angle) / 5.f), 1, 6);


	// 방향 보간
	for (int32 i = 0; i <= Steps; ++i)
	{
		float Alpha = (float)i / Steps;
		FVector Dir = FMath::Lerp(m_PrevDir, CurDir, Alpha).GetSafeNormal();



		FVector Start = CurStart;


		FVector End = Start + Dir * BladeLength;



		sweepAttack(Start, End);

	}

	m_PrevDir = CurDir;
}

void UC_AttackComponent::tickThrustTrace()
{
	FVector CurStart = m_Owner->getTraceStartLocation();
	FVector CurEnd = m_Owner->getTraceEndLocation();
	FVector Forward = m_Owner->GetActorForwardVector();

	float BladeLength = FVector::Distance(CurStart, CurEnd);

	FVector End = CurStart + Forward * BladeLength;

	sweepAttack(CurStart, End);
}

