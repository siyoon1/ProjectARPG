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

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
	FPostProcessSettings m_PostProcessSettings;

public:
	void startSprintEffect();
	void stopSprintEffect();
	
};
