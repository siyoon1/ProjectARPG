// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_CombatCharacter.h"
#include "ProjectARPG/Interface/C_ExecutionTarget.h"
#include "ProjectARPG/Enums/C_ExecutionTypes.h"
#include "C_EnemyCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossCombatStateChanged, AC_EnemyCharacter*, Boss, bool, bInCombat);
DECLARE_MULTICAST_DELEGATE(FOnAttackFinished);
DECLARE_MULTICAST_DELEGATE(FOnStepBackFinished);
DECLARE_MULTICAST_DELEGATE(FOnGuardFinished);

UENUM(BlueprintType)
enum class E_EnemyTier : uint8
{
	Weak,
	Soldier,
	MiniBoss,
	Boss
};

UENUM(BlueprintType)
enum class E_CombatIntent : uint8
{
	None,
	Attack,
	Guard,
	Reposition,
	Chase
};

UENUM(BlueprintType)
enum class E_EnemyActionState : uint8
{
	Idle,     
	Executing,  
	Cooldown    
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
	float fThrustWeight = 0.2f;

	UPROPERTY(EditAnywhere)
	float fHeavyWeight = 0.3f;

	UPROPERTY(EditAnywhere)
	float fActionInterval = 0.25f;

	UPROPERTY(EditAnywhere)
	float fGuardMinTime = 0.6f;

	UPROPERTY(EditAnywhere)
	float fGuardMaxTime = 1.8f;

	UPROPERTY(EditAnywhere)
	float fGuardReleaseDelay = 0.3f;

	UPROPERTY(EditAnywhere)
	float fPreferredRange = 180.f;
};

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTARPG_API AC_EnemyCharacter : public AC_CombatCharacter, public IC_ExecutionTarget
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWidgetComponent> m_wHpBarCom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	E_EnemyTier m_eEnemyTier;

	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> m_ExecutionVFX;

	UPROPERTY()
	TObjectPtr<class UC_DetectComponent> m_DetectCom;

	


	UPROPERTY()
	AC_CombatCharacter* m_pPlayer{};

	bool m_bCanBeExecuted = false;
	bool m_bInCombat = false;
	float m_fGuardStartTime = 0.f;
	float m_nextActionTime = 0.f;

	FTimerHandle m_guardHandle;
	FTimerHandle m_actionCooldownHandle;

protected:
	E_CombatIntent m_CombatIntent;
	TMap<FName, FS_AttackRuntimeState> m_AttackStates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UC_EnemyAttackComponent> m_EnemyAttackComp;

	UPROPERTY()
	E_EnemyActionState m_EnemyActionState = E_EnemyActionState::Idle;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TMap<E_EnemyTier, FS_EnemyCombatProfile> m_CombatProfiles;

	FS_EnemyCombatProfile m_CurrentCombatProfile;

	
public:
	UPROPERTY(BlueprintAssignable, Category = "BossStatus")
	FOnBossCombatStateChanged m_onBossStateChanged;

	FOnAttackFinished m_onAttackFinished;
	FOnStepBackFinished m_onStepBackFinished;
	FOnGuardFinished m_onGuardFinished;

	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	class UC_ExecutionReactionData* m_ReactionData;
	

private:
	void applyCombatProfile();
	float getDistToTarget() const;
	void applyExecutionFacing(const struct FS_ExecutionContext& Context);

public:
	AC_EnemyCharacter();

protected:
	void BeginPlay() override;
	

public:
	void Tick(float DeltaTime) override;

	inline UC_EnemyAttackComponent* getEnemyAttackComponent() const
	{
		return m_EnemyAttackComp;
	}

	// Runtime 사용 조회
	const FS_AttackRuntimeState* getAttackRuntimeState(FName Row) const;

	// 사용 기록
	void markAttackUsed(FName Row, float Cooldown);

	// UI 관련
	void showHpBar(bool bShow);

	void showExecutionVFX(bool bShow);

	void onCombatStarted();
	void onCombatEnded();
	float getNextActionTime() const;
	FS_EnemyCombatProfile& getCombatProfile();

	//인살 관련
	virtual bool canBeExecuted(E_ExecutionType Type) const override;
	virtual void onExecutionStarted(APawn* ExecutionInstigator, const FS_ExecutionContext& Context) override;
	virtual void onExecutionFinished(APawn* ExecutionInstigator) override;
	virtual void setExecutionHintVisible(bool bVisible) override;


	void setCanBeExecuted(bool bCan);

	bool canBeExecuted() const;

	bool isAnawareOfPlayer() const;

	//체간 붕괴

	void onPostureBroken() override;
	void onPostureBroken_Internal() override;

	void setInCombat(bool bCombat);

	

	//행동 실행 API
	bool canDecideAction() const;
	void beginAction();
	void finishAction(float fCooldown);
	void onActionCooldownFinished();

	bool playAttackByRow(FName AttackRow);

	bool tryAttack();

	// 실제 공격 실행
	bool playAttack(const FS_AttackData* AttackData);

	void endAttack();

	bool playStepBack();
	void endStepBack();

	bool startGuard();
	bool canReleaseGuard() const;
	void endGuard();

	bool guardForDuration(float fTime);


	UFUNCTION(BlueprintCallable)
	bool isGuard() const;

	UFUNCTION(BlueprintCallable)
	bool isCombat() const;

	void onParryFinished();

	UFUNCTION(BlueprintCallable)
	bool isBoss() const;


	//죽음
	void onDeath() override;

	//데미지 처리
	void takeDamage_Implementation(float fDamage, float fPostureDamage, bool bGuardSuccess, AActor* pAttacker) override;

	void tryParry_Implementation(AActor* ParryOwner) override;
};
