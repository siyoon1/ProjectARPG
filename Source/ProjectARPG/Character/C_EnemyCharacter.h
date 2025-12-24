// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_CombatCharacter.h"
#include "C_EnemyCharacter.generated.h"

UENUM(BlueprintType)
enum class E_EnemyCombatAction : uint8
{
	None,
	Attack,
	Guard,
	Parry,
	Wait
};

UENUM(BlueprintType)
enum class E_EnemyAttackType : uint8
{
	Light      UMETA(DisplayName = "Light"),
	Heavy      UMETA(DisplayName = "Heavy"),
	Thrust     UMETA(DisplayName = "Thrust")
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

	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> m_ExecutionVFX;

	UPROPERTY()
	TObjectPtr<class UC_DetectComponent> m_DetectCom;

	bool m_bIsExecutingAction = false;

	FTimerHandle m_guardHandle;

protected:
	E_EnemyAttackType m_eCurrentAttackType;
	E_EnemyCombatAction m_eCurrentAction;

	

private:
	// 가드
	void guardForDuration(float fTime);

protected:
	void BeginPlay() override;

	//행동 분기
	E_EnemyCombatAction decideCombatAction() const;

	//공격 타입 분기
	E_EnemyAttackType decideAttackType() const;
	

public:
	// UI 관련
	void showHpBar(bool bShow);

	void showExecutionVFX(bool bShow);

	//인살 관련

	void setCanBeExecuted(bool bCan);

	bool canBeExecuted() const;

	bool isAnawareOfPlayer() const;

	//체간 붕괴

	void onPostureBroken() override;

	//행동
	void executeCombatAction();

	bool isExecutingAction() const;

	bool isActionFinished() const;

	void endGuard();
	void endAttack();

	void attack();

	//데미지 처리
	void takeDamage_Implementation(float fDamage, float fPostureDamage, bool bGuardSuccess, AActor* pAttacker) override;

	
};
