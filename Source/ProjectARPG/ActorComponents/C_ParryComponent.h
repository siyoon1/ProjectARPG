// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectARPG/Data/C_AttackData.h"
#include "C_ParryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParryWindowEnded);

UENUM()
enum class E_ParryResult : uint8
{
	None,
	Failed,
	Guarded,
	Parried
};

USTRUCT()
struct FS_ParryResult
{
	GENERATED_BODY()

	E_ParryResult Result = E_ParryResult::None;

	E_ParryDirection Direction = E_ParryDirection::None;

	float PostureDamageToAttacker = 0.f;
	float PostureDamageToDefender = 0.f;
};


USTRUCT()
struct FS_ParryContext
{
	GENERATED_BODY()

	bool bWindowOpen = false;
	float EndTime = 0.f;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_ParryComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	FS_ParryContext m_ParryContext;
	
	FTimerHandle m_ParryTimerHandle;

public:
	UPROPERTY(BlueprintAssignable)
	FOnParryWindowEnded m_OnParryWindowEnded;

public:	
	// Sets default values for this component's properties
	UC_ParryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 패링 입력시 호출
	void startParryWindow(float Duration);

	// 공격 히트시 호출
	FS_ParryResult evaluateParry(const FS_AttackData& AttackData, AActor* Attacker) const;

	bool isParryWindowOpen() const;

	void endParryWindow();

};
