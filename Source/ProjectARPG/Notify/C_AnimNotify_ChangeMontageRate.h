// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "C_AnimNotify_ChangeMontageRate.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_AnimNotify_ChangeMontageRate : public UAnimNotify
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NewRate = 1.0f;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
        const FAnimNotifyEventReference& EventReference) override;
	
};
