// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectARPGGameMode.h"
#include "ProjectARPGCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectARPGGameMode::AProjectARPGGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
