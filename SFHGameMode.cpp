// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SFHGameMode.h"
#include "SFHHUD.h"
#include "SFHCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASFHGameMode::ASFHGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ASFHHUD::StaticClass();
}
