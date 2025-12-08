// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_GrappleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_GrappleComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	bool m_bIsPulling = false;

	FVector m_vPullDir{};
	FVector m_vOwnerPos{};
	FVector m_vTargetPos{};

	float m_fGrappleSpeed = 3000.f;


	float m_fElapsed{};
	float m_fDuration{};

	TObjectPtr<class AC_PlayerCharacter> m_pOwner = nullptr;

	TObjectPtr<class AC_GrapplePoint> m_pCurrentTarget = nullptr;


private:
	void endPull();

	bool isInView(class UCameraComponent* pCamera, AC_GrapplePoint* pTarget);

	AC_GrapplePoint* findBestGrapplePoint();

public:	
	// Sets default values for this component's properties
	UC_GrappleComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void tryStartGrapple();

	void startPull(AC_GrapplePoint* pTarget);
};
