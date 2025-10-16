// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_BaseCharacter.h"
#include "C_CombatCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API AC_CombatCharacter : public AC_BaseCharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* m_pPostureStatsTable;

	struct FS_PostureStats* m_sPostureStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Posture", meta = (AllowPrivateAccess = "true"))
	float m_fCurrentPosture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Posture", meta = (AllowPrivateAccess = "true"))
	FName m_sPostureRowName;

public:
	AC_CombatCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
