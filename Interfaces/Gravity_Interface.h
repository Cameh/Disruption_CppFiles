// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Gravity_Interface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UGravity_Interface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SFH_API IGravity_Interface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
		bool RequestGravityChange();
};
