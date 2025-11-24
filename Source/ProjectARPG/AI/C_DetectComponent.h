// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_DetectComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_DetectComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Detect")
	float m_DetectDist = 800.f;

	UPROPERTY(EditAnywhere, Category = "Detect")
	float m_MinDotThreshold = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Detect")
	TSubclassOf<class AC_PlayerCharacter> m_PlayerClass;

	UPROPERTY(EditAnywhere, Category = "Detect")
	AActor* m_DetectedTarget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Detect|General")
	float m_DetectInterval = 0.1f;

	float m_TimeSinceLastDetect = 0.f;

private:
	void detectTarget();

	bool checkDist(AC_PlayerCharacter* pPlayer);
	bool checkFOV(AC_PlayerCharacter* pPlayer);
	bool checkLineOfSight(AC_PlayerCharacter* pPlayer);
	float getAdjustedDetectDist(AC_PlayerCharacter* pPlayer);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Sets default values for this component's properties
	UC_DetectComponent();

	UFUNCTION()
	void forceDetect(AActor* pTarget);

	inline AActor* getDetectedTarget() const { return m_DetectedTarget; }

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
