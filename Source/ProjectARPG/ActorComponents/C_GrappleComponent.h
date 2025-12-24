// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_GrappleComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrappleStateChanged, bool, bCanGrapple, AC_GrapplePoint*, pTarget);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_GrappleComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> m_pGrappleStartMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> m_pGrappleEndMontage;

private:
	bool m_bCachedCanGrapple = false;
	bool m_bIsPulling = false;


	FVector m_vPullDir{};
	FVector m_vOwnerPos{};
	FVector m_vTargetPos{};

	float m_fGrappleSpeed = 3000.f;


	float m_fElapsed{};
	float m_fDuration{};

	TObjectPtr<class AC_PlayerCharacter> m_pOwner = nullptr;

	TObjectPtr<class AC_GrapplePoint> m_pCurrentTarget = nullptr;



	// 로프 관련

	bool m_bIsFiringRope = false;

	float m_fRopeFireAlpha = 0.f;

	float m_fRopeFireSpeed = 2.f;

	float m_DesiredLength{};

	FVector m_vRopeFireStart{};
	FVector m_vRopeFireEnd{};

public:
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* m_pGrappleBeamSystem{};

	UPROPERTY()
	class UNiagaraComponent* m_pBeamComp{};

	UPROPERTY(BlueprintAssignable)
	FOnGrappleStateChanged m_onGrappleStateChanged;

private:
	void endPull();

	bool isInView(class UCameraComponent* pCamera, AC_GrapplePoint* pTarget);

	bool canGrapple(AC_GrapplePoint*& outTarget);

	bool hasLineOfSight(AC_GrapplePoint* pTarget) const;

	AC_GrapplePoint* findBestGrapplePoint();

	void startFireRope(AC_GrapplePoint* pTarget);

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

	bool isPulling() const;

};
