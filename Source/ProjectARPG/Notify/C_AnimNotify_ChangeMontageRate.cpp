// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimNotify_ChangeMontageRate.h"

void UC_AnimNotify_ChangeMontageRate::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp)
        return;

    if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
    {
        if (UAnimMontage* Montage =
            AnimInst->GetCurrentActiveMontage())
        {
            AnimInst->Montage_SetPlayRate(Montage, NewRate);
        }
    }
}
