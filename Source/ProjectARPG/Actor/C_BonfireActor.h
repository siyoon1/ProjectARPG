// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectARPG/Interface/C_Interactable.h"
#include "C_BonfireActor.generated.h"

UCLASS()
class PROJECTARPG_API AC_BonfireActor : public AActor, public IC_Interactable
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Bonfire")
	FName m_BonfireID;

	UPROPERTY(VisibleAnywhere, Category = "Bonfire")
	bool m_bActivated = false;

private:
	void activateBonfire();
	void setCheckPoint();
	void saveGame();
	void healPlayer(class AC_PlayerCharacter* Player);
	
public:	
	// Sets default values for this actor's properties
	AC_BonfireActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void interact_Implementation(AActor* Interactor) override;

	FName getBonfireID() const;

};
