// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Reveal_Interface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UReveal_Interface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SFH_API IReveal_Interface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//classes using this interface must implement RequestReveal
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hidden")
		bool RequestReveal();
};
