// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_LockOnComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnTarget, AActor*, Target);




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_LockOnComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float m_DetectRadius = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	TEnumAsByte<ECollisionChannel> m_DetectChannel = ECC_GameTraceChannel3;

	UPROPERTY()
	TWeakObjectPtr<AActor> m_CurrentTarget;

	UPROPERTY()
	APawn* m_OwnerPawn;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnLockOnTarget OnLockOnStarted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnLockOnTarget OnLockOnEnded;

public:	
	// Sets default values for this component's properties
	UC_LockOnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void toggleLockOn();
	void clearLockOn();

	bool isLockOn() const;
	AActor* getCurrentTarget() const;

	bool getLockOnRotation(const FVector& CameraLocation, FRotator& OutRot) const;

private:
	AActor* findTarget();
		
};
