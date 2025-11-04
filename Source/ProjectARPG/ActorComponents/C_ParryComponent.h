// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_ParryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSuccessParry, AActor*, ParryOwner, AActor*, ParriedTarget);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_ParryComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	bool m_bCanParry = false;
	
	FTimerHandle m_ParryTimerHandle;

public:
	FOnSuccessParry m_OnSuccessParry;

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

	bool tryParry(AActor* pAttacker);

	void startParryWindow(float fCanTime);

	void endParryWindow();

};
