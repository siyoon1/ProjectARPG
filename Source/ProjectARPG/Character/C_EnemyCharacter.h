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

protected:
	void BeginPlay() override;

public:
	void showHpBar(bool bShow);

	void showExecutionVFX(bool bShow);

	void setCanBeExecuted(bool bCan);

	bool canBeExecuted() const;

	void onPostureBroken() override;

	void attack();
};
