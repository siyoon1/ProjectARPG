// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_ExecutionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_ExecutionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimMontage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> m_pAttackerMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimMontage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> m_pEnemyMontage;

public:	
	// Sets default values for this component's properties
	UC_ExecutionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void performExecution(APawn* pInstigator, APawn* pVictim);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void onBecomeExecutable(APawn* pVictim);
	void triggerExecution(APawn* pVictim);

		
};
