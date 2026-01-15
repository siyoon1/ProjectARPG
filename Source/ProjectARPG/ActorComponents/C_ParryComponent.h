// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectARPG/Data/C_AttackData.h"
#include "C_ParryComponent.generated.h"

USTRUCT()
struct FS_ParryContext
{
	GENERATED_BODY()

	bool bActive = false;

	const FS_AttackData* CurrentAttackData = nullptr;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_ParryComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	FS_ParryContext m_CurrentParry;

	bool m_bCanParry = false;
	
	FTimerHandle m_ParryTimerHandle;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimMontage")
	TObjectPtr<UAnimMontage> m_ParryOwnerMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimMontage")
	TObjectPtr<UAnimMontage> m_ParriedTargetMontage;

public:	
	// Sets default values for this component's properties
	UC_ParryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void openParry(const FS_AttackData& AttackData);
	void closeParry();

	bool canParry() const;


	void startParryWindow(float fCanTime);

	void endParryWindow();

	bool isCanParry() const;

};
