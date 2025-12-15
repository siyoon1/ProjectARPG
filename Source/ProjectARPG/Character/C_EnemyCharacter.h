// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_CombatCharacter.h"
#include "C_EnemyCharacter.generated.h"

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

	

protected:
	void BeginPlay() override;

	

public:
	// UI 관련
	void showHpBar(bool bShow);

	void showExecutionVFX(bool bShow);

	//인살 관련

	void setCanBeExecuted(bool bCan);

	bool canBeExecuted() const;

	bool isUnawareOfPlayer() const;

	//체간 붕괴

	void onPostureBroken() override;

	//공격
	void attack();

	//데미지 처리
	void takeDamage_Implementation(float fDamage, float fPostureDamage, bool bGuardSuccess, AActor* pAttacker) override;

	
};
