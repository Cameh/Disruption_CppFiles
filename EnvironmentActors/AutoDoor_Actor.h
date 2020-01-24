// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GravitySwitch_Actor.h"
#include "Components/BillboardComponent.h"
#include "AutoDoor_Actor.generated.h"

UCLASS()
class SFH_API AAutoDoor_Actor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAutoDoor_Actor();

	UFUNCTION()
		void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapRevert(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DoorOpen")
	bool bDoorOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DoorOpen")
		bool bGravityChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GravitySwitch")
		AGravitySwitch_Actor* GravSwitchRef;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
		UBoxComponent* TraceCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
		UBoxComponent* RevertTraceCollision;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
		USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
		UBillboardComponent* BillboardComp;

	bool bTraceEnabled;
	bool bRevertEnabled;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
