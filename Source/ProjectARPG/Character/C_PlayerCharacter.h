// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_CombatCharacter.h"
#include "C_PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API AC_PlayerCharacter : public AC_CombatCharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> m_pSpringArm;
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> m_pCamera;

public:
	AC_PlayerCharacter();

	
};
