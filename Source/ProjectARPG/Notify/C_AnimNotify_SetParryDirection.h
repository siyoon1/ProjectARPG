// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
#include "C_AnimNotify_SetParryDirection.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_AnimNotify_SetParryDirection : public UAnimNotify
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	E_ParryDirection m_NewDirection = E_ParryDirection::None;


public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
		const FAnimNotifyEventReference& EventReference) override;
	
};
