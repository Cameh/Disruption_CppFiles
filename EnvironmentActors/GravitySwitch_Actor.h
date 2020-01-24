// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../SFHCharacter.h"
#include "GravitySwitch_Actor.generated.h"


UCLASS()
class SFH_API AGravitySwitch_Actor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGravitySwitch_Actor();

	void SwitchGravity(AActor* ActorRef);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interface)
	TScriptInterface<UInterface> GravityInterface;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
		USceneComponent* SceneRoot;

	ASFHCharacter* CharacterRef;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
