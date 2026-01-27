// Fill out your copyright notice in the Description page of Project Settings.

#include "C_CombatCharacter.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "ProjectARPG/Animation/C_CombatAnim.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "ProjectARPG/ActorComponents/C_CombatStatComponent.h"
#include "ProjectARPG/ActorComponents/C_AttackComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AC_CombatCharacter::AC_CombatCharacter()
{
	m_StatComp = CreateDefaultSubobject<UC_CombatStatComponent>(TEXT("StatComp"));
	m_AttackComp = CreateDefaultSubobject<UC_AttackComponent>(TEXT("AttackComp"));
	m_ParryCom = CreateDefaultSubobject<UC_ParryComponent>(TEXT("ParryComp"));
}

void AC_CombatCharacter::BeginPlay()
{
	Super::BeginPlay();


	if (!m_pTraceStart)
	{
		m_pTraceStart = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("TraceStart")));
	}

	if (!m_pTraceEnd)
	{
		m_pTraceEnd = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("TraceEnd")));
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		m_CamMgr = Cast<AC_PlayerCameraManager>(PC->PlayerCameraManager);
	}

	if (m_StatComp)
		m_StatComp->m_OnPostureBroken.AddUObject(this, &AC_CombatCharacter::onPostureBroken);

	if (m_ParryCom)
		m_ParryCom->m_OnParryWindowEnded.AddDynamic(this, &AC_CombatCharacter::onParryWindowEnded);
}


void AC_CombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AC_CombatCharacter::applyHitStop(float fSlowlate, float fDuration)
{
	if (m_bHitStopActive)
		return;

	if (!canAct())
		return;

	UAnimInstance* pAnim = GetMesh()->GetAnimInstance();

	if (!pAnim)
		return;

	UAnimMontage* pMontage{};

	pMontage = pAnim->GetCurrentActiveMontage();

	if (!pMontage)
		return;

	m_bHitStopActive = true;
	
	m_lastMontage = pMontage;

	m_fOriginalPlayRate = pAnim->Montage_GetPlayRate(pMontage);


	pAnim->Montage_SetPlayRate(pMontage, fSlowlate);
	UE_LOG(LogTemp, Warning, TEXT("HitStop start: rate=%.3f duration=%.3f"), fSlowlate, fDuration);

	GetWorldTimerManager().ClearTimer(m_hitStopTimerHandle);
	GetWorldTimerManager().SetTimer
	(
		m_hitStopTimerHandle,
		this,
		&AC_CombatCharacter::endHitStop,
		fDuration,
		false
	);

	
}

void AC_CombatCharacter::endHitStop()
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;

	UAnimMontage* Montage = Anim->GetCurrentActiveMontage();
	if (!Montage) return;

	if (m_lastMontage)
	{
		Anim->Montage_SetPlayRate(m_lastMontage, m_fOriginalPlayRate);
	}
	UE_LOG(LogTemp, Warning, TEXT("HitStop END: rate=%.3f"), m_fOriginalPlayRate);

	m_lastMontage = nullptr;
	m_bHitStopActive = false;
}

void AC_CombatCharacter::setRuntimeParryDir(E_ParryDirection eDir)
{
	m_RuntimeParryDir = eDir;
}

void AC_CombatCharacter::applyHitFeedback(E_HitResult HitResult, AActor* Attacker)
{
	if (AC_CombatCharacter* AttackerChar =
		Cast<AC_CombatCharacter>(Attacker))
	{
		AttackerChar->applyAttackerHitFeedback(HitResult, Attacker);
	}

	m_CamMgr->playHitCameraShake(0.2f);
}

void AC_CombatCharacter::applyAttackerHitFeedback(E_HitResult HitResult, AActor* Attacker)
{

	switch (HitResult)
	{
	case E_HitResult::Normal:
		applyHitStop(0.05f, 0.02f);
		m_CamMgr->playHitCameraShake(0.2f);
		break;

	case E_HitResult::Guarded:
		applyHitStop(0.03f, 0.015f);
		break;

	case E_HitResult::PostureBroken:
		applyHitStop(0.1f, 0.08f);
		break;
	}
	

}

void AC_CombatCharacter::onHitConfirmed(E_HitResult Result, AActor* Attacker)
{
	switch (Result)
	{
	case E_HitResult::Normal:
	case E_HitResult::PostureBroken:
	{
		E_Direction HitDir = getHitDirection(Attacker);
		playHitMontage(HitDir);
		break;
	}

	case E_HitResult::Guarded:
	
		break;
	}

	applyHitFeedback(Result, Attacker);
	applyHitPushBack(Attacker->GetActorLocation(), Result);
}

void AC_CombatCharacter::applyHitPushBack(const FVector& From, E_HitResult Result)
{
	FVector PushDir = (GetActorLocation() - From).GetSafeNormal();

	float PushStrength = 0.f;

	switch (Result)
	{
	case E_HitResult::Normal:
		PushStrength = 220.f;
		break;

	case E_HitResult::Guarded:
		PushStrength = 80.f;   // 막아도 밀림
		break;

	case E_HitResult::PostureBroken:
		PushStrength = 400.f;
		break;
	}

	LaunchCharacter(
		PushDir * PushStrength,
		true,
		false
	);
}


void AC_CombatCharacter::startAttack(const FS_AttackData& AttackData)
{
	enterCombatMode(E_CombatMode::Attacking, E_ActionState::Locked);

	m_pCurrentAttackData = &AttackData;

	m_fAttackDamage = AttackData.Combat.Damage;
	m_fPostureDamage = AttackData.Combat.PostureDamage;
	m_fGuardPushBack = AttackData.Combat.GuardPushBack;

	if (m_AttackComp)
		m_AttackComp->startAttack(AttackData);
}

void AC_CombatCharacter::endAttack()
{
	if (m_AttackComp)
		m_AttackComp->endAttack();

	m_pCurrentAttackData = nullptr;
}

void AC_CombatCharacter::setCurrentAttackRow(FName RowName)
{
	m_CurrentAttackRow = RowName;
}

FName AC_CombatCharacter::getCurrentAttackRow() const
{
	return m_CurrentAttackRow;
}

const UDataTable* AC_CombatCharacter::getAttackDataTable() const
{
	return m_pAttackDataTable;
}

void AC_CombatCharacter::setActionState(E_ActionState eNewState)
{
	if (m_ActionState != eNewState)
		m_ActionState = eNewState;
}

E_ActionState AC_CombatCharacter::getActionState() const
{
	return m_ActionState;
}

void AC_CombatCharacter::setCombatState(E_CombatState eNewState)
{
	if (m_eState != eNewState)
		m_eState = eNewState;
}

E_CombatState AC_CombatCharacter::getCombatState() const
{
	return m_eState;
}

FVector AC_CombatCharacter::getTraceStartLocation() const
{
	return m_pTraceStart ? m_pTraceStart->GetComponentLocation()
		: GetActorLocation();
}

FVector AC_CombatCharacter::getTraceEndLocation() const
{
	return m_pTraceEnd ? m_pTraceEnd->GetComponentLocation()
		: GetActorLocation();
}

void AC_CombatCharacter::enterCombatMode(E_CombatMode NewMode, E_ActionState NewActionState)
{
	m_CombatMode = NewMode;
	m_ActionState = NewActionState;
}

const FS_AttackData* AC_CombatCharacter::getAttackData(FName RowName) const
{
	if (RowName.IsNone())
		return nullptr;

	if (!m_pAttackDataTable)
		return nullptr;


	return m_pAttackDataTable->FindRow<FS_AttackData>(RowName, TEXT("getAttackData"));
}

const FS_AttackData* AC_CombatCharacter::getCurrentAttackData() const
{
	return m_pCurrentAttackData;
}

bool AC_CombatCharacter::startGuard()
{
	if (!canAct())
		return false;

	
	enterCombatMode(E_CombatMode::Guarding, E_ActionState::Locked);
	m_bIsGuarding = true;
	m_fGuardStartTime = GetWorld()->GetTimeSeconds();

	return true;
}

void AC_CombatCharacter::endGuard()
{
	m_bIsGuarding = false;

	enterCombatMode(E_CombatMode::None, E_ActionState::Free);
}

bool AC_CombatCharacter::isGuardingFront(AActor* pAttacker) const
{
	if (!pAttacker)
		return false;

	FVector vAttacker = (pAttacker->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector vFront = GetActorForwardVector();


	float fDot = FVector::DotProduct(vFront, vAttacker);

	UE_LOG(LogTemp, Warning,
		TEXT("[GuardCheck] Guard=%d Dot=%.3f Attacker=%s"),
		isGuard(),
		fDot,
		*pAttacker->GetName());

	return (fDot > -0.3f);
}

void AC_CombatCharacter::setGuard(bool bSet)
{
	m_bIsGuarding = bSet;
}

bool AC_CombatCharacter::isGuard() const
{
	return m_bIsGuarding;
}

float AC_CombatCharacter::getGuardPostureMultiplier() const
{
	return 0.7f;
}

bool AC_CombatCharacter::canAct() const
{
	return m_ActionState == E_ActionState::Free;
}

bool AC_CombatCharacter::isInvincibleAgainst(AActor* pAttacker) const
{
	if (isDead())
		return true;

	//인살 연출 중
	if (m_CombatMode == E_CombatMode::Executing)
		return true;

	return false;
}

float AC_CombatCharacter::getHp() const
{
	return m_StatComp->getCurrentHp();
}


float AC_CombatCharacter::getPosture() const
{
	return m_StatComp->getCurrentPosture();
}


void AC_CombatCharacter::takeDamage_Implementation(float Damage, float PostureDamage, E_HitResult HitResult, AActor* pAttacker)
{
	if (isInvincibleAgainst(pAttacker))
		return;

	float FinalDamage = Damage;
	float FinalPostureDamage = PostureDamage;
	

	switch (HitResult)
	{
	case E_HitResult::Guarded:
		FinalDamage *= 0.2f;
		FinalPostureDamage *= getGuardPostureMultiplier();
		break;

	case E_HitResult::PostureBroken:
		break;

	default:
		break;
	}

	m_StatComp->applyDamage(FinalDamage, FinalPostureDamage);
	
}

void AC_CombatCharacter::onPostureBroken()
{

	enterCombatMode(E_CombatMode::None, E_ActionState::Stunned);

	onPostureBroken_Internal();

}

void AC_CombatCharacter::onPostureBroken_Internal()
{
	
}

void AC_CombatCharacter::playHitMontage(E_Direction eDir)
{
	if (UC_CombatAnim* pAnim = Cast<UC_CombatAnim>(GetMesh()->GetAnimInstance()))
	{
		pAnim->playHitMontage(eDir);
	}
}

E_Direction AC_CombatCharacter::getHitDirection(AActor* pAttacker)
{
	FVector AttackDir = (pAttacker->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector Forward = GetActorForwardVector();

	float Dot = FVector::DotProduct(Forward, AttackDir);
	float CrossZ = FVector::CrossProduct(Forward, AttackDir).Z;

	if (Dot > 0.5f)      return E_Direction::Forward;
	if (Dot < -0.5f)     return E_Direction::Backward;
	if (CrossZ > 0.f)    return E_Direction::Right;


	return E_Direction::Left;

}

void AC_CombatCharacter::endParried()
{
	enterCombatMode(E_CombatMode::None, E_ActionState::Free);
}

void AC_CombatCharacter::onParried_Implementation(AActor* ParryOwner)
{
	if (m_AttackComp)
		m_AttackComp->endAttack();

	enterCombatMode(E_CombatMode::None, E_ActionState::Stunned);

	const float DefaultParryStun = 0.4f;

	GetWorldTimerManager().SetTimer(
		m_ParriedTimer,
		this,
		&AC_CombatCharacter::endParried,
		DefaultParryStun,
		false
	);


	
}

void AC_CombatCharacter::applyAttack(const FS_AttackData& AttackData)
{
	// TODO: 실제 공격 처리
}

UC_ParryComponent* AC_CombatCharacter::getParryComponent() const
{
	return m_ParryCom;
}

void AC_CombatCharacter::onParryWindowEnded()
{
	if (m_CombatMode != E_CombatMode::Parrying)
		return;

	enterCombatMode(E_CombatMode::None, E_ActionState::Free);
	setCombatState(E_CombatState::Idle);

	UE_LOG(LogTemp, Warning,
		TEXT("[%s] Parry failed → back to Idle"),
		*GetName());
}

void AC_CombatCharacter::onDeath()
{
	enterCombatMode(E_CombatMode::None, E_ActionState::Dead);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	 
}

bool AC_CombatCharacter::isDead() const
{
	return m_ActionState == E_ActionState::Dead;
}


