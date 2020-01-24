// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsGrab_Actor.h"
#include "Containers/UnrealString.h"
#include "SFHCharacter.h"

// Sets default values
APhysicsGrab_Actor::APhysicsGrab_Actor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PhysicsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsMesh"));
	PhysicsMesh->SetupAttachment(RootComponent);
	RootComponent = PhysicsMesh;

	TraceCollision = CreateDefaultSubobject<USphereComponent>(TEXT("TraceCollision"));
	TraceCollision->SetupAttachment(PhysicsMesh);

	PhysConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysConstraint"));
	PhysConstraint->SetupAttachment(PhysicsMesh);

}

// Called when the game starts or when spawned
void APhysicsGrab_Actor::BeginPlay()
{
	Super::BeginPlay();

	TraceCollision->OnComponentBeginOverlap.AddDynamic(this, &APhysicsGrab_Actor::OnOverlap);
	TraceCollision->OnComponentEndOverlap.AddDynamic(this, &APhysicsGrab_Actor::OnOverlapEnd);

	//Set default overlap settings depending on if gravity is enabled
	if (PhysicsMesh->IsGravityEnabled())
	{
		PhysicsMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	}

	else
	{
		PhysicsMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);
	}
	
}

// Called every frame
void APhysicsGrab_Actor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//Allow Blueprints to take over for these
bool APhysicsGrab_Actor::RequestPickup_Implementation()
{
	return true;

}

bool APhysicsGrab_Actor::RequestHidden_Implementation()
{
	return true;

}

bool APhysicsGrab_Actor::RequestReveal_Implementation()
{
	return true;

}


bool APhysicsGrab_Actor::RequestGravityChange_Implementation()
{
	//Switch Gravity
	if (PhysicsMesh->IsGravityEnabled())
	{
		PhysicsMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);
		PhysicsMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);
		PhysicsMesh->SetEnableGravity(false);

	}
	else
	{
		PhysicsMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
		PhysicsMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Ignore);
		PhysicsMesh->SetEnableGravity(true);
	}
	return true;

}

void APhysicsGrab_Actor::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UWorld* WorldRef = GetWorld();
	ASFHCharacter* CharacterRef = Cast<ASFHCharacter>(WorldRef->GetFirstPlayerController()->GetCharacter());
	if (CharacterRef)
	{
		if (OtherActor == CharacterRef)
		{
			//Allow player to interact with multiple versions of this object
			CharacterRef->bEnableHoverCheck = true;

			CharacterRef->OverlapCount += 1;
		}

	}
}

void APhysicsGrab_Actor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UWorld* WorldRef = GetWorld();
	ASFHCharacter* CharacterRef = Cast<ASFHCharacter>(WorldRef->GetFirstPlayerController()->GetCharacter());
	if (CharacterRef)
	{
		if (OtherActor == CharacterRef)
		{
			//Remove from player hover check
			CharacterRef->OverlapCount -= 1;

			if (CharacterRef->OverlapCount == 0)
			{
				CharacterRef->bEnableHoverCheck = false;
			}

			if (CharacterRef->WidgetRef)
			{
				CharacterRef->RemoveIconWidget();
			}
		}

	}
}