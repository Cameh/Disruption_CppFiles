// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "EnvironmentActors/PhysicsGrab_Actor.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Interfaces/Gravity_Interface.h"
#include "Engine.h"
#include "SFHCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class ASFHCharacter : public ACharacter, public IGravity_Interface
{
	GENERATED_BODY()

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneCaptureComponent2D* ReverseCameraComponent;





public:
	ASFHCharacter();

	virtual void Tick(float DeltaTime) override;

	void RemoveIconWidget();

protected:
	virtual void BeginPlay();

	void LerpCamPos(float TargetHeight, float DeltaTime);

	void Sprint();

	void RevertSprint();

	float MinSprint;

	float MaxSprint;

	/* Handle to manage the timer */
	FTimerHandle CamVisTimerHandle;

	float CamHeight;

	bool bCamNeedUpdating;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float TraceDistance;

	/*-------Interface Declarations-------*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interface)
	TScriptInterface<UInterface> InteractionInterface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interface)
	TScriptInterface<UInterface> HoverInterface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interface)
	TScriptInterface<UInterface> PickupInterface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interface)
	TScriptInterface<UInterface> HiddenInterface;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gravity")
		bool RequestGravityChange();
		virtual bool RequestGravityChange_Implementation() override;

	/*-------/Interface Declarations-------*/

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
	USpringArmComponent* ScreenSpringArm;

	FVector ScreenArmTopLoc;

	FVector ScreenArmBotLoc;

	bool bEnableSlow;

	bool bEnableHoverCheck;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interact)
	bool bIconCreated;

	bool bIsZeroG;

	int OverlapCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interact)
	bool bIsInteractItem;

	bool bCheckFloor;

	bool bIsPushing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gravity)
	bool bSwitchedMovement;

	//Handle Physics Handle Variables
	bool bPhysicsHandleActive;

	bool bIsHeld;

	float OtherItemLoc;

	float HoldItemDist;

	FRotator OtherItemRot;

	FVector HandleLoc;

	FRotator HandleRot;

	UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Physics)
	APhysicsGrab_Actor* PhysObject;

	EObjectTypeQuery* PhysObjTypes;

	UStaticMeshComponent* PhysComp;

	bool RotatingObj;

	FRotator LastCamRot;

	//Icon Widget
	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UUserWidget> IconWidgetBPReference;

	UUserWidget* WidgetRef;

	APlayerController* PlayerController;

	//Shakes
	UPROPERTY(EditAnywhere)
		TSubclassOf<UCameraShake> GravityLandShake;

protected:

	
	/** Fires a projectile. */
	void OnFire();


	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	void ForceForward();

	void SwitchMovement();

	void EnableSlowDown();

	void RevertSlowDown();

	void SlowDown(float TargetDamping, float InterpSpeed, float DeltaTime);

	void HoverCheck();

	void HoldPhysicsObject();

	void ReleaseClick();

	void PushObj();

	void PullObj();



	void RotObj();

	void RotObjRelease();

	void RotObjTick();

	void AddCustomControllerYawInput(float Val);

	void AddCustomControllerPitchInput(float Val);

	virtual void Landed(const FHitResult& Hit) override;

	void CheckForFloor();

	void CamVisCheck();
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	


public:

	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

