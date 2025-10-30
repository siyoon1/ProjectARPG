// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "C_PlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API AC_PlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

private:
	bool m_bIsExecuting = false;
	bool m_bIsReturningFOV = false;
	bool m_bIsSprinting = false;
	FTimerHandle m_TimerHandle_Reset;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
	FPostProcessSettings m_PostProcessSettings;

public:
	AC_PlayerCameraManager();

	void Tick(float DeltaSeconds) override;

	void startSprintEffect();
	void stopSprintEffect();
	void executionEffect(float fLength);
	
};
