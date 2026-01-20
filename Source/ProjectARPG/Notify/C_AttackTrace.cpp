// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AttackTrace.h"
#include "ProjectARPG/ActorComponents/C_AttackComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"



void UC_AttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	if (APawn* pPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		if (AC_CombatCharacter* pOwner = Cast<AC_CombatCharacter>(pPawn))
		{

			if (UC_AttackComponent* AttackComp =
				pOwner->FindComponentByClass<UC_AttackComponent>())
			{

				FName Row = pOwner->getCurrentAttackRow();
				if (!Row.IsNone())
				{
					const FS_AttackData* Data = pOwner->getAttackData(Row);
					if (Data)
					{
						AttackComp->startAttack(*Data);
					}
				}


				AttackComp->startTrace();
			}

			
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[AttackStart] Notify called"));


}

void UC_AttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	if (APawn* pPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		if (AC_CombatCharacter* pOwner = Cast<AC_CombatCharacter>(pPawn))
		{
			if (UC_AttackComponent* AttackComp = pOwner->FindComponentByClass<UC_AttackComponent>())
			{
				AttackComp->tickTrace();
			}
		}
	}
}

void UC_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	if (APawn* pPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		if (AC_CombatCharacter* pOwner = Cast<AC_CombatCharacter>(pPawn))
		{
			if (UC_AttackComponent* AttackComp =
				pOwner->FindComponentByClass<UC_AttackComponent>())
			{
				AttackComp->stopTrace();
			}
		}
	}
}
