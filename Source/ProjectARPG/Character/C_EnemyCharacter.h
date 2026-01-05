// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_CombatCharacter.h"
#include "C_EnemyCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossCombatStateChanged, AC_EnemyCharacter*, Boss, bool, bInCombat);

UENUM(BlueprintType)
enum class E_EnemyTier : uint8
{
	Weak,
	Soldier,
	MiniBoss,
	Boss
};

UENUM(BlueprintType)
enum class E_EnemyCombatAction : uint8
{
	None	UMETA(DisplayName = "None"),
	Attack	UMETA(DisplayName = "Attack"),
	Guard	UMETA(DisplayName = "Guard"),
	Wait	UMETA(DisplayName = "Wait")
};

UENUM(BlueprintType)
enum class E_EnemyAttackType : uint8
{
	Light      UMETA(DisplayName = "Light"),
	Heavy      UMETA(DisplayName = "Heavy"),
	Thrust     UMETA(DisplayName = "Thrust"),
	Sweep	   UMETA(DisplayName = "Sweep"),
	GuardBreak UMETA(DisplayName = "GuardBreak")
};


USTRUCT(BlueprintType)
struct FS_EnemyCombatProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float fAttackProbability = 0.7f;

	UPROPERTY(EditAnywhere)
	float fGuardProbability = 0.3f;

	UPROPERTY(EditAnywhere)
	float fThrustRatio = 0.2f;

	UPROPERTY(EditAnywhere)
	float fActionInterval = 0.25f;

	UPROPERTY(EditAnywhere)
	float fGuardDuration = 0.6f;

	UPROPERTY(EditAnywhere)
	float fAttackRange = 180.f;

	UPROPERTY(EditAnywhere)
	bool bCanAutoParry = false;
};

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTARPG_API AC_EnemyCharacter : public AC_CombatCharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWidgetComponent> m_wHpBarCom;

	bool m_bCanbeExcuted = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	E_EnemyTier m_eEnemyTier;

	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> m_ExecutionVFX;

	UPROPERTY()
	TObjectPtr<class UC_DetectComponent> m_DetectCom;

	UPROPERTY()
	AC_CombatCharacter* m_pPlayer{};


	bool m_bInCombat = false;

	bool m_bIsExecutingAction = false;
	bool m_bActionStarted = false;

	float m_fAttackRange = 180.f;

	float m_nextActionTime = 0.f;

	FTimerHandle m_guardHandle;

protected:
	E_EnemyAttackType m_eCurrentAttackType;
	E_EnemyCombatAction m_eCurrentAction;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TMap<E_EnemyTier, FS_EnemyCombatProfile> m_CombatProfiles;

	FS_EnemyCombatProfile m_CurrentCombatProfile;

public:
	UPROPERTY(BlueprintAssignable, Category = "BossStatus")
	FOnBossCombatStateChanged m_onBossStateChanged;

	

private:
	void applyCombatProfile();

protected:
	void BeginPlay() override;

	//공격 타입 분기
	E_EnemyAttackType decideAttackType() const;

	bool isPlayerAttacking() const;
	

public:
	void Tick(float DeltaTime) override;

	// UI 관련
	void showHpBar(bool bShow);

	void showExecutionVFX(bool bShow);

	void onCombatStarted();
	void onCombatEnded();

	//인살 관련

	void setCanBeExecuted(bool bCan);

	bool canBeExecuted() const;

	bool isAnawareOfPlayer() const;

	//체간 붕괴

	void onPostureBroken() override;

	void setInCombat(bool bCombat);

	void endGuard();
	void endAttack();

	//행동 실행 API

	void attack();

	void guardForDuration(float fTime);

	bool isExecutingAction() const;

	UFUNCTION(BlueprintCallable)
	bool isGuard() const;

	void onParryFinished();

	UFUNCTION(BlueprintCallable)
	bool isBoss() const;

	//인살
	void onExecuted() override;

	//죽음
	void onDeath() override;

	//데미지 처리
	void takeDamage_Implementation(float fDamage, float fPostureDamage, bool bGuardSuccess, AActor* pAttacker) override;

	void tryParry_Implementation(AActor* ParryOwner) override;

	
};
