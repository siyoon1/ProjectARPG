// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GrapplePoint.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "Components/SphereComponent.h"

// Sets default values
AC_GrapplePoint::AC_GrapplePoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_pSphere = CreateDefaultSubobject<USphereComponent>("Sphere");

}

// Called when the game starts or when spawned
void AC_GrapplePoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AC_GrapplePoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AC_GrapplePoint::isOverlapSphere()
{
	TArray<AActor*> Overlaps{};


	m_pSphere->GetOverlappingActors(Overlaps, AC_PlayerCharacter::StaticClass());

	for (AActor* pAct : Overlaps)
	{
		if (AC_PlayerCharacter* pOwner = Cast<AC_PlayerCharacter>(pAct))
		{
			return true;
		}

	}

	return false;
}

