// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SFHCharacter.h"
#include "SFHProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Containers/UnrealString.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Interfaces/Interact_Interface.h"
#include "Interfaces/Hover_Interface.h"
#include "Interfaces/Pickup_Interface.h"
#include "EnvironmentActors/Tube_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "DrawDebugHelpers.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ASFHCharacter

ASFHCharacter::ASFHCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 55.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	MinSprint = 300.f;
	MaxSprint = 500.f;

	//Set line trace distance
	TraceDistance = 200.f;

	HoldItemDist = 100.f;

	//Set Screen Arm Locations
	ScreenArmTopLoc = FVector(-23.573912f, 14.472038f, -2.281138f);
	ScreenArmBotLoc = FVector(-14.075188f, -11.889559f, -39.214542f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 0.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	ReverseCameraComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ReverseCamera1"));
	ReverseCameraComponent->SetupAttachment(FirstPersonCameraComponent);
	ReverseCameraComponent->SetRelativeRotation(FRotator(0.f, 0.f, 180.f));
	ReverseCameraComponent->MaxViewDistanceOverride = 600.f;
	ReverseCameraComponent->CaptureSource = SCS_FinalColorLDR;

	GetCapsuleComponent()->SetEnableGravity(false);

	//Create Physics Handle
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

	//Create screen spring arm
	ScreenSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ScreenSpringArm"));
	ScreenSpringArm->SetupAttachment(ReverseCameraComponent);
	ScreenSpringArm->SetRelativeLocationAndRotation(ScreenArmBotLoc, FRotator(-24.999987f, 145.000061, 0.000012f));
	ScreenSpringArm->TargetArmLength = -30.f;
	ScreenSpringArm->bEnableCameraLag = true;
	ScreenSpringArm->bEnableCameraRotationLag = true;
	ScreenSpringArm->CameraLagSpeed = 50.f;
	ScreenSpringArm->CameraRotationLagSpeed = 70.f;

}

void ASFHCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeGameOnly());

	bIsZeroG = true;

	PlayerController = GetWorld()->GetFirstPlayerController();

}

void ASFHCharacter::Tick(float DeltaTime)
{

	//GEngine->AddOnScreenDebugMessage(-36, 1.f, FColor::Red, FString::Printf(TEXT("Overlap Count: %d"), OverlapCount));

	if (bEnableSlow)
	{
		SlowDown(5.f, 100.f, DeltaTime);
	}

	if (bEnableHoverCheck)
	{
		HoverCheck();
	}
	

	//Handle Physics Handle Setup
	if (bPhysicsHandleActive && !bIsPushing)
	{
		if (!bSwitchedMovement)
		{
			HandleLoc = (FirstPersonCameraComponent->GetComponentLocation() + (FirstPersonCameraComponent->GetForwardVector() * HoldItemDist));
		}

		else
		{
			HandleLoc = (ReverseCameraComponent->GetComponentLocation() + (ReverseCameraComponent->GetForwardVector() * HoldItemDist));

		}

		//To be safe check the handle exists
		if (PhysicsHandle)
		{
			PhysicsHandle->SetTargetLocationAndRotation(HandleLoc, HandleRot);
		}
	}

	if (RotatingObj)
	{
		RotObjTick();
	}

	if (bCheckFloor)
	{
		CheckForFloor();
	}

	if (bIsPushing)
	{
		ForceForward();
	}

	if (bCamNeedUpdating)
	{
		LerpCamPos(CamHeight, DeltaTime);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASFHCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &ASFHCharacter::OnFire);
	PlayerInputComponent->BindAction("Grab", IE_Released, this, &ASFHCharacter::ReleaseClick);

	// Bind ScrollUp event
	PlayerInputComponent->BindAction("PushObj", IE_Pressed, this, &ASFHCharacter::PushObj);

	// Bind ScrollDown event
	PlayerInputComponent->BindAction("PullObj", IE_Pressed, this, &ASFHCharacter::PullObj);

	// Bind RotObj event
	PlayerInputComponent->BindAction("RotObj", IE_Pressed, this, &ASFHCharacter::RotObj);
	PlayerInputComponent->BindAction("RotObj", IE_Released, this, &ASFHCharacter::RotObjRelease);

	// Bind slow down event
	PlayerInputComponent->BindAction("SlowDownSpeed", IE_Pressed, this, &ASFHCharacter::EnableSlowDown);
	PlayerInputComponent->BindAction("SlowDownSpeed", IE_Released, this, &ASFHCharacter::RevertSlowDown);

	// Bind sprint event
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASFHCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASFHCharacter::RevertSprint);

	// Bind fire event
	PlayerInputComponent->BindAction("SwitchMovement", IE_Pressed, this, &ASFHCharacter::SwitchMovement);


	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ASFHCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASFHCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ASFHCharacter::AddCustomControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASFHCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ASFHCharacter::AddCustomControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASFHCharacter::LookUpAtRate);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//Key Press Events

void ASFHCharacter::OnFire()
{
	
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;

	//Re-initialize hit info
	FHitResult RV_Hit(ForceInit);

	//call GetWorld() from within an actor extending class
	if (!bSwitchedMovement)
	{
		GetWorld()->LineTraceSingleByChannel(RV_Hit, FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetForwardVector() * TraceDistance + FirstPersonCameraComponent->GetComponentLocation(), ECC_WorldStatic, RV_TraceParams);
	}
	else
	{
		GetWorld()->LineTraceSingleByChannel(RV_Hit, ReverseCameraComponent->GetComponentLocation(), ReverseCameraComponent->GetForwardVector() * TraceDistance + ReverseCameraComponent->GetComponentLocation(), ECC_WorldStatic, RV_TraceParams);
	}

	RV_Hit.bBlockingHit; //did hit something? (bool)
	RV_Hit.GetActor(); //the hit actor if there is one
	RV_Hit.ImpactPoint;  //FVector
	RV_Hit.Location;
	RV_Hit.ImpactNormal;  //FVector
	RV_Hit.BoneName;

	if (RV_Hit.GetActor())
	{
		

		IInteract_Interface* InteractionInterface = Cast<IInteract_Interface>(RV_Hit.GetActor());
		if (InteractionInterface)
		{
			//Run Request Interaction function from the Interface
			InteractionInterface->Execute_RequestInteraction(RV_Hit.GetActor());

			if (WidgetRef)
			{
				RemoveIconWidget();
			}
		}

		IPickup_Interface* PickupInterface = Cast<IPickup_Interface>(RV_Hit.GetActor());
		if (PickupInterface)
		{
			APhysicsGrab_Actor* PhysRef = Cast<APhysicsGrab_Actor>(RV_Hit.GetActor());

			bPhysicsHandleActive = true;
			bIsHeld = true;

			OtherItemLoc = FVector(GetActorLocation() - RV_Hit.Location).Size();

			PhysObject = PhysRef;

			PhysicsHandle->GrabComponentAtLocation(PhysRef->PhysicsMesh, RV_Hit.BoneName, RV_Hit.Location);

			PhysRef->PhysicsMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

			PhysComp = PhysRef->PhysicsMesh;

			PhysRef->PhysConstraint->SetAngularSwing1Limit(ACM_Locked, 0.f);
			PhysRef->PhysConstraint->SetAngularSwing2Limit(ACM_Locked, 0.f);
			PhysRef->PhysConstraint->SetAngularTwistLimit(ACM_Locked, 0.f);

			if(WidgetRef)
			{
				RemoveIconWidget();
			}
		}

		else if (!InteractionInterface && !PickupInterface && !WidgetRef)
		{
			if (bIsZeroG)
			{
				bIsPushing = true;
			}
		}
	}

	else if (!InteractionInterface && !PickupInterface && !PickupInterface)
	{
		if (bIsZeroG)
		{
			bIsPushing = true;
		}
	}
}

void ASFHCharacter::ReleaseClick()
{
	bPhysicsHandleActive = false;
	bIsPushing = false;
	
	if (PhysicsHandle)
	{
		PhysicsHandle->ReleaseComponent();
		if (PhysObject)
		{
			PhysObject->PhysConstraint->SetAngularSwing1Limit(ACM_Free, 0.f);
			PhysObject->PhysConstraint->SetAngularSwing2Limit(ACM_Free, 0.f);
			PhysObject->PhysConstraint->SetAngularTwistLimit(ACM_Free, 0.f);
		}
		InteractionInterface = nullptr;
		PickupInterface = nullptr;
		RotObjRelease();
	}


	if (bIsHeld)
	{
		if (PhysComp)
		{
			PhysComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
			if (PhysObject)
			{
				PhysObject->PhysConstraint->SetAngularSwing1Limit(ACM_Free, 0.f);
				PhysObject->PhysConstraint->SetAngularSwing2Limit(ACM_Free, 0.f);
				PhysObject->PhysConstraint->SetAngularTwistLimit(ACM_Free, 0.f);
			}
			InteractionInterface = nullptr;
			PickupInterface = nullptr;
			bIsHeld = false;
			RotObjRelease();
		}
	}
}


/*----------------------------------Movement Functions----------------------------------*/

void ASFHCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ASFHCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ASFHCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASFHCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//Custom Yaw Control
void ASFHCharacter::AddCustomControllerYawInput(float Val)
{
	if (PlayerController)
	{
		APlayerController* const PC = CastChecked<APlayerController>(PlayerController);
		PC->AddYawInput(Val);
		//RootComponent->AddRelativeRotation(FRotator(0.f, Val, 0.f));
	}
}

//Custom Pitch Control
void ASFHCharacter::AddCustomControllerPitchInput(float Val)
{
	if (PlayerController)
	{
		APlayerController* const PC = CastChecked<APlayerController>(PlayerController);
		if (!bSwitchedMovement)
		{
			PC->AddPitchInput(Val);
		}
		
		else
		{
			PC->AddPitchInput(Val * -1.f);
		}
		//RootComponent->AddRelativeRotation(FRotator(Val, 0.f, 0.f));
	}
}

void ASFHCharacter::ForceForward()
{
	if (!bSwitchedMovement)
	{
		GetCapsuleComponent()->AddImpulse(FirstPersonCameraComponent->GetForwardVector() * 1000.f);
	}

	else
	{
		GetCapsuleComponent()->AddImpulse(FirstPersonCameraComponent->GetForwardVector() * -1000.f);
	}
}



/*----------------------------------Slow Speed Functions----------------------------------*/

//Handle Right Click Event
void ASFHCharacter::EnableSlowDown()
{
	bEnableSlow = true;

	//Throw Phys Object if held
	if (bIsHeld)
	{
		PhysicsHandle->ReleaseComponent();
		if (PhysObject)
		{
			PhysObject->PhysConstraint->SetAngularSwing1Limit(ACM_Free, 0.f);
			PhysObject->PhysConstraint->SetAngularSwing2Limit(ACM_Free, 0.f);
			PhysObject->PhysConstraint->SetAngularTwistLimit(ACM_Free, 0.f);
		}

		if (!bSwitchedMovement)
		{
			PhysComp->AddImpulse(FirstPersonCameraComponent->GetForwardVector() * 1500.f, "NAME_None", true);
		}

		else
		{
			PhysComp->AddImpulse(ReverseCameraComponent->GetForwardVector() * 1500.f, "NAME_None", true);
		}
		PhysComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
		bIsHeld = false;
		bPhysicsHandleActive = false;
		InteractionInterface = nullptr;
		PickupInterface = nullptr;
		WidgetRef = nullptr;
		RotObjRelease();
	}
}

void ASFHCharacter::RevertSlowDown()
{
	bEnableSlow = false;
	GetCapsuleComponent()->SetLinearDamping(0.2f);
}

void ASFHCharacter::SlowDown(float TargetDamping, float InterpSpeed, float DeltaTime)
{
	GetCapsuleComponent()->SetLinearDamping(FMath::FInterpConstantTo(GetCapsuleComponent()->GetLinearDamping(), TargetDamping, InterpSpeed, DeltaTime));
}


/*----------------------------------Gravity Switch Function----------------------------------*/
void ASFHCharacter::SwitchMovement()
{
	if (bIsZeroG)
	{
		if (!bSwitchedMovement)
		{
			bSwitchedMovement = true;
			ScreenSpringArm->SetRelativeLocation(ScreenArmTopLoc);
			GetWorld()->GetTimerManager().SetTimer(CamVisTimerHandle, this, &ASFHCharacter::CamVisCheck, 0.1, true);
		}
		else
		{
			bSwitchedMovement = false;
			ScreenSpringArm->SetRelativeLocation(ScreenArmBotLoc);
			GetWorld()->GetTimerManager().ClearTimer(CamVisTimerHandle);
		}
	}
}

/*----------------------------------Look At Object Functions----------------------------------*/
void ASFHCharacter::HoverCheck()
{
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;

	//Re-initialize hit info
	FHitResult RV_Hit(ForceInit);

	if (!bSwitchedMovement)
	{
		GetWorld()->LineTraceSingleByChannel(RV_Hit, FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetForwardVector() * TraceDistance + FirstPersonCameraComponent->GetComponentLocation(), ECC_WorldStatic, RV_TraceParams);
	}

	else
	{
		GetWorld()->LineTraceSingleByChannel(RV_Hit, ReverseCameraComponent->GetComponentLocation(), ReverseCameraComponent->GetForwardVector() * TraceDistance + ReverseCameraComponent->GetComponentLocation(), ECC_WorldStatic, RV_TraceParams);
	}

	RV_Hit.bBlockingHit;
	RV_Hit.GetActor();
	RV_Hit.ImpactPoint;
	RV_Hit.ImpactNormal;

	if (RV_Hit.GetActor())
	{
		IHover_Interface* HoverInterface = Cast<IHover_Interface>(RV_Hit.GetActor());

		if (HoverInterface)
		{
			if (!bIconCreated)
			{
				//Run Request Hover from the Interface
				HoverInterface->Execute_RequestHover(RV_Hit.GetActor());
				UUserWidget* WidgetInstance = CreateWidget<UUserWidget>(GetWorld()->GetFirstPlayerController(), IconWidgetBPReference);
				WidgetRef = WidgetInstance;

				if (!bSwitchedMovement)
				{
					WidgetInstance->AddToViewport();
				}
				bIconCreated = true;
			}
		}

		IPickup_Interface* PickupInterface = Cast<IPickup_Interface>(RV_Hit.GetActor());
		if (PickupInterface)
		{
			if (!bIconCreated && !bIsHeld)
			{
				bIsInteractItem = true;
				UUserWidget* WidgetInstance = CreateWidget<UUserWidget>(GetWorld()->GetFirstPlayerController(), IconWidgetBPReference);
				WidgetRef = WidgetInstance;

				//Handle camera screen widget icon through blueprints
				PickupInterface->Execute_RequestPickup(RV_Hit.GetActor());

				if(!bSwitchedMovement)
				{
					WidgetInstance->AddToViewport();
				}
				bIconCreated = true;
			}
		}

		else if (bIconCreated && !HoverInterface && !PickupInterface)
		{
			if (WidgetRef)
			{
				RemoveIconWidget();
			}
		}
	}

	else if (bIconCreated && !HoverInterface && !PickupInterface)
	{
		if (WidgetRef)
		{
			RemoveIconWidget();
		}
	}
}

/*----------------------------------Holding Object Functions----------------------------------*/
void ASFHCharacter::PushObj()
{
	if (HoldItemDist <= 100.f)
	{
		HoldItemDist += 10.f;
	}
}

void ASFHCharacter::PullObj()
{
	if (HoldItemDist >= 50.f)
	{
		HoldItemDist -= 10.f;
	}
}

void ASFHCharacter::RemoveIconWidget()
{
	if (WidgetRef->IsInViewport())
	{
		WidgetRef->RemoveFromParent();
	}
	WidgetRef = nullptr;
	bIconCreated = false;
	bIsInteractItem = false;
}

void ASFHCharacter::RotObj()
{
	if (PhysObject && bIsHeld)
	{
		LastCamRot = GetWorld()->GetFirstPlayerController()->GetControlRotation();
		GetWorld()->GetFirstPlayerController()->SetControlRotation(PhysObject->GetActorRotation());
		RotatingObj = true;
		FirstPersonCameraComponent->bUsePawnControlRotation = false;
	}
}

void ASFHCharacter::RotObjRelease()
{
	if (RotatingObj)
	{
		GetWorld()->GetFirstPlayerController()->SetControlRotation(LastCamRot);
		RotatingObj = false;
		FirstPersonCameraComponent->bUsePawnControlRotation = true;
	}
}

void ASFHCharacter::RotObjTick()
{
	if (bIsHeld)
	{
		if (PhysObject)
		{
			PhysObject->SetActorRelativeRotation(FQuat(GetWorld()->GetFirstPlayerController()->GetControlRotation()));
		}
	}
}

/*----------------------------------Returned To Gravity Events----------------------------------*/

void ASFHCharacter::Landed(const FHitResult& Hit)
{
	// make sure the parent class still does its things
	Super::Landed(Hit);

	if (!bIsZeroG)
	{
		//Play landing shake
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayCameraShake(GravityLandShake, 0.5f);

		if (bSwitchedMovement)
		{
			bSwitchedMovement = false;
			ScreenSpringArm->SetRelativeLocation(ScreenArmBotLoc);
			GetWorld()->GetTimerManager().ClearTimer(CamVisTimerHandle);
		}

	}
}

void ASFHCharacter::CheckForFloor()
{
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;

	//Re-initialize hit info
	FHitResult RV_Hit(ForceInit);

	//call GetWorld() from within an actor extending class
	GetWorld()->LineTraceSingleByChannel(RV_Hit, GetCapsuleComponent()->GetComponentLocation(), FVector(GetCapsuleComponent()->GetComponentLocation().X, GetCapsuleComponent()->GetComponentLocation().Y, GetCapsuleComponent()->GetComponentLocation().Z - 80.f), ECC_WorldStatic, RV_TraceParams);

	RV_Hit.bBlockingHit; //did hit something? (bool)
	RV_Hit.GetActor(); //the hit actor if there is one
	RV_Hit.ImpactPoint;  //FVector
	RV_Hit.ImpactNormal;  //FVector

	if (RV_Hit.GetActor())
	{
		GetCapsuleComponent()->SetSimulatePhysics(false);
		bCheckFloor = false;
	}
}

void ASFHCharacter::CamVisCheck()
{
	/*----------------------------------Handle Back Sphere Trace----------------------------------*/
	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// start and end locations
	FVector SweepStart = FVector((ReverseCameraComponent->GetForwardVector() * 700.f) + ReverseCameraComponent->GetComponentLocation());
	FVector SweepEnd = FVector((ReverseCameraComponent->GetForwardVector() * 800.f) + ReverseCameraComponent->GetComponentLocation());

	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(700.0f);

	// draw collision sphere
	//DrawDebugSphere(GetWorld(), SweepStart, MyColSphere.GetSphereRadius(), 15, FColor::Purple, false, 0.2f);

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, SweepStart, SweepEnd, FQuat::Identity, ECC_GameTraceChannel2, MyColSphere);

	if (isHit)
	{
		// loop through TArray
		for (auto& Hit : OutHits)
		{
			IHidden_Interface* HiddenInterface = Cast<IHidden_Interface>(Hit.GetActor());
			if (HiddenInterface)
			{
				HiddenInterface->Execute_RequestHidden(Hit.GetActor());
			}
		}
	}



	/*----------------------------------Handle Front Sphere Trace----------------------------------*/
	
	// create tarray for hit results
	TArray<FHitResult> FrontOutHits;

	// start and end locations
	FVector FrontSweepStart = FVector((FirstPersonCameraComponent->GetForwardVector() * 700.f) + FirstPersonCameraComponent->GetComponentLocation());
	FVector FrontSweepEnd = FVector((FirstPersonCameraComponent->GetForwardVector() * 800.f) + FirstPersonCameraComponent->GetComponentLocation());

	// create a collision sphere
	FCollisionShape FrontColSphere = FCollisionShape::MakeSphere(700.0f);
	//FCollisionShape FrontColSphere = FCollisionShape::MakeBox(FVector(600.f, 500.f, 500.f));

	// draw collision sphere
	//DrawDebugSphere(GetWorld(), FrontSweepStart, FrontColSphere.GetSphereRadius(), 15, FColor::Purple, false, 0.2f);
	//DrawDebugBox(GetWorld(), FrontSweepStart, FVector(600.f, 500.f, 500.f), FColor::Red, false, 0.2f);

	// check if something got hit in the sweep
	bool isHitFront = GetWorld()->SweepMultiByChannel(FrontOutHits, FrontSweepStart, FrontSweepEnd, FQuat::Identity, ECC_GameTraceChannel2, FrontColSphere);

	if (isHitFront)
	{
		// loop through TArray
		for (auto& Hit : FrontOutHits)
		{
			IHidden_Interface* HiddenInterface = Cast<IHidden_Interface>(Hit.GetActor());
			if (HiddenInterface)
			{
				HiddenInterface->Execute_RequestReveal(Hit.GetActor());
			}
		}
	}
	
}

/*--------Deal with Gravity Change and Overlaps--------*/
bool ASFHCharacter::RequestGravityChange_Implementation()
{
	if (bIsZeroG)
	{
		GetCapsuleComponent()->SetEnableGravity(true);
		bUseControllerRotationYaw = true;
		GetCapsuleComponent()->SetCapsuleHalfHeight(60.f);
		GetCapsuleComponent()->SetCapsuleRadius(30.f);
		bCheckFloor = true;
		bIsZeroG = false;
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Ignore);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

		CamHeight = 55.f;

		bCamNeedUpdating = true;
	}
	else
	{
		GetCapsuleComponent()->SetEnableGravity(false);
		GetCapsuleComponent()->SetSimulatePhysics(true);
		bUseControllerRotationYaw = false;
		GetCapsuleComponent()->SetCapsuleHalfHeight(25.f);
		GetCapsuleComponent()->SetCapsuleRadius(25.f);
		bCheckFloor = false;
		bIsZeroG = true;
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);

		CamHeight = 0.f;

		bCamNeedUpdating = true;
	}
	return true;

}

void ASFHCharacter::LerpCamPos(float TargetHeight, float DeltaTime)
{
	GetFirstPersonCameraComponent()->SetRelativeLocation(FVector(0.f, 0.f, (FMath::FInterpConstantTo(GetFirstPersonCameraComponent()->RelativeLocation.Z, TargetHeight, 100.f, DeltaTime))));

	if (GetFirstPersonCameraComponent()->RelativeLocation.Z == TargetHeight)
	{
		bCamNeedUpdating = false;
	}
}

void ASFHCharacter::Sprint()
{
	if (!bIsZeroG)
	{
		GetCharacterMovement()->MaxWalkSpeed = MaxSprint;
	}
}

void ASFHCharacter::RevertSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = MinSprint;
}