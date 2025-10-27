// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ExecutionComponent.h"
#include "NiagaraComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"

// Sets default values for this component's properties
UC_ExecutionComponent::UC_ExecutionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

}


// Called when the game starts
void UC_ExecutionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		if (m_ExecutionVFX)
		{
			m_ExecutionVFX->AttachToComponent(
				OwnerChar->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				FName("spine_03") // 또는 pelvis, chest 등 이펙트 위치에 맞는 소켓
			);
		}
	}

	if (m_ExecutionVFX)
	{
		m_ExecutionVFX->SetVisibility(false);
		m_ExecutionVFX->Deactivate();

	}
	
}

void UC_ExecutionComponent::performExcution(APawn* pInstigator, APawn* pVictim)
{

}


// Called every frame
void UC_ExecutionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_ExecutionComponent::onBecomeExcutable(APawn* pVictim)
{
	if (m_ExecutionVFX)
	{
		m_ExecutionVFX->SetVisibility(true);
		m_ExecutionVFX->Activate(true);
	
	}
		
}

void UC_ExecutionComponent::tirggerExcution(APawn* pVictim)
{

}

