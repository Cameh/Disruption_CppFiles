// Fill out your copyright notice in the Description page of Project Settings.


#include "GravitySwitch_Actor.h"
#include "Engine/World.h"

// Sets default values
AGravitySwitch_Actor::AGravitySwitch_Actor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneRoot;

}

// Called when the game starts or when spawned
void AGravitySwitch_Actor::BeginPlay()
{
	Super::BeginPlay();

	UWorld* WorldRef = GetWorld();
	CharacterRef = Cast<ASFHCharacter>(WorldRef->GetFirstPlayerController()->GetCharacter());
	
}

// Called every frame
void AGravitySwitch_Actor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGravitySwitch_Actor::SwitchGravity(AActor* ActorRef)
{
	//Call the interface event on the passed through actor
	IGravity_Interface* GravityInterface = Cast<IGravity_Interface>(ActorRef);
	if (GravityInterface)
	{
		GravityInterface->Execute_RequestGravityChange(ActorRef);
	}
}
