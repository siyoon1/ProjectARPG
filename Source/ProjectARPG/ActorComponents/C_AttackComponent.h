// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectARPG/Data/C_AttackData.h"
#include "C_AttackComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_AttackComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	bool m_bPostureBrokenByParry = false;

protected:
	UPROPERTY()
	class AC_CombatCharacter* m_Owner;

	const FS_AttackData* m_CurrentAttackData = nullptr;

	bool m_bTracing = false;

	UPROPERTY()
	TSet<AActor*> m_HitActors;

	FVector m_PrevStart;
	FVector m_PrevEnd;
	FVector m_PrevDir;

	float m_BladeLength = 0.f;

public:	
	// Sets default values for this component's properties
	UC_AttackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void startAttack(const FS_AttackData& AttackData);

	void endAttack();

	void startTrace();
	void stopTrace();
	void tickTrace();

	inline bool isAttacking() const { return m_CurrentAttackData != nullptr; }

	inline bool isParryBroken() const { return m_bPostureBrokenByParry; }

private:
	void applyHit(AActor* HitActor);

	// Sweep 공통
	void sweepAttack(const FVector& Start, const FVector& End);

	// 베기 전용
	void tickArcTrace();

	// 찌르기 전용
	void tickThrustTrace();
		
};
