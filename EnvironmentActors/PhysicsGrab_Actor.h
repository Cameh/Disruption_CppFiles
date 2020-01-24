// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Interfaces/Pickup_Interface.h"
#include "Interfaces/Hidden_Interface.h"
#include "Interfaces/Gravity_Interface.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsGrab_Actor.generated.h"

UCLASS()
class SFH_API APhysicsGrab_Actor : public AActor, public IPickup_Interface, public IHidden_Interface, public IGravity_Interface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APhysicsGrab_Actor();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
		UStaticMeshComponent* PhysicsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Physics")
		UPhysicsConstraintComponent* PhysConstraint;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
		USceneComponent* SceneRoot;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
		USphereComponent* TraceCollision;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Interface Events
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
		bool RequestPickup();
		virtual bool RequestPickup_Implementation() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hidden")
		bool RequestHidden();
		virtual bool RequestHidden_Implementation() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hidden")
		bool RequestReveal();
		virtual bool RequestReveal_Implementation() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gravity")
		bool RequestGravityChange();
		virtual bool RequestGravityChange_Implementation() override;


	UFUNCTION()
		void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
