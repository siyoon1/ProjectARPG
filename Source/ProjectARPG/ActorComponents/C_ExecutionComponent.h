// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_ExecutionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_ExecutionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UC_ExecutionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void performExcution(APawn* pInstigator, APawn* pVictim);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void onBecomeExcutable(APawn* pVictim);
	void tirggerExcution(APawn* pVictim);

		
};
