// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Interfaces/Interact_Interface.h"
#include "Interfaces/Hover_Interface.h"
#include "Components/BoxComponent.h"
#include "DoorPanel_Actor.generated.h"

UCLASS()
class SFH_API ADoorPanel_Actor : public AActor, public IInteract_Interface, public IHover_Interface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoorPanel_Actor();

	UFUNCTION()
		void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
		USceneComponent* SceneRoot;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
		UStaticMeshComponent* PanelMesh;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
		UBoxComponent* TraceCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	bool bIsSwitchEnabled;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
		bool RequestInteraction();
		virtual bool RequestInteraction_Implementation() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hover")
		bool RequestHover();
		virtual bool RequestHover_Implementation() override;

	/*UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
		void RequestHoverCheck();
		virtual void RequestHoverCheck_Implementation() override;*/

};
