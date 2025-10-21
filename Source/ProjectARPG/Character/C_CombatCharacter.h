// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_BaseCharacter.h"
#include "ProjectARPG/Interface/C_CombatInterface.h"
#include "C_CombatCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API AC_CombatCharacter : public AC_BaseCharacter, public IC_CombatInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* m_pPostureStatsTable;

	struct FS_PostureStats* m_sPostureStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Posture", meta = (AllowPrivateAccess = "true"))
	FName m_sPostureRowName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Posture", meta = (AllowPrivateAccess = "true"))
	float m_fCurrentPosture = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hp", meta = (AllowPrivateAccess = "true"))
	float m_fMaxHp = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hp", meta = (AllowPrivateAccess = "true"))
	float m_fCurrnetHp = 0.f;

public:
	AC_CombatCharacter();

	UFUNCTION(BlueprintCallable)
	void setHp(float fHp);
	UFUNCTION(BlueprintCallable)
	float getHp() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
