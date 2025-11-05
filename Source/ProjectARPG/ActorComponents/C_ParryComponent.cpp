// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ParryComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"

// Sets default values for this component's properties
UC_ParryComponent::UC_ParryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_ParryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UC_ParryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UC_ParryComponent::tryParry(AActor* pAttacker)
{
	if (!m_bCanParry)
		return false;

	m_bCanParry = false;

	AActor* pParryOwner = GetOwner(); // 이 컴포넌트가 붙어있는 쪽 (패리 당한 or 시도 받은 쪽)
	AActor* pParriedTarget = pAttacker; // 공격자 (패리 시도한 캐릭터)

	if (pParryOwner && pParriedTarget)
	{
		// 기존: 자기 자신(Enemy)의 델리게이트만 호출됨
		m_OnSuccessParry.Broadcast(pParriedTarget, pParryOwner);

		// 추가: 패리 성공자(Player) 쪽도 강제로 onParrySuccess 실행
		if (AC_CombatCharacter* pParryOwnerChar = Cast<AC_CombatCharacter>(pParryOwner))
		{
			// 상대가 Player라면 그의 델리게이트도 울리게 한다
			if (AC_CombatCharacter* pAttackerChar = Cast<AC_CombatCharacter>(pParriedTarget))
			{
				if (pAttackerChar->m_pParryCom)
				{
					pAttackerChar->m_pParryCom->m_OnSuccessParry.Broadcast(pParriedTarget, pParryOwner);
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("TryParry | Owner=%s | Attacker=%s"),
			*pParryOwner->GetName(),
			pParriedTarget ? *pParriedTarget->GetName() : TEXT("NULL"));

		return true;
	}

	return false;
}

void UC_ParryComponent::startParryWindow(float fCanTime)
{
	m_bCanParry = true;

	GetWorld()->GetTimerManager().ClearTimer(m_ParryTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(m_ParryTimerHandle, this, &UC_ParryComponent::endParryWindow,
		fCanTime, false);
	UE_LOG(LogTemp, Warning, TEXT("%s ParryWindow Start (%.2fs)"), *GetOwner()->GetName(), fCanTime);

}

void UC_ParryComponent::endParryWindow()
{
	m_bCanParry = false;
	UE_LOG(LogTemp, Warning, TEXT("%s ParryWindow End"), *GetOwner()->GetName());
}

