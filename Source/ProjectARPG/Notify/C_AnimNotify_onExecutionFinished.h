// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "C_AnimNotify_onExecutionFinished.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_AnimNotify_onExecutionFinished : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
		const FAnimNotifyEventReference& EventReference) override;
	
};
