// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectARPG/Data/C_AttackData.h"
#include "C_EnemyAttackComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_EnemyAttackComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	class AC_EnemyCharacter* m_OwnerEnemy;

	UDataTable* m_AttackDataTable;

public:	
	// Sets default values for this component's properties
	UC_EnemyAttackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	bool tryExecuteAttack(float fDist);
	void getAttackCandidates(float fDist, TArray<FName>& OutCandidates) const;
	

	bool canUseAttack(FName Row) const;
	bool isAttackInRange(const FS_AttackData& Data, float fDist) const;

private:
	bool executeAttack(FName Row);
	bool decideNextAttack(float fDist, FName& OutAttackRow) const;
};
