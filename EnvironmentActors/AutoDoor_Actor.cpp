// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoDoor_Actor.h"


// Sets default values
AAutoDoor_Actor::AAutoDoor_Actor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneRoot;

	TraceCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("TraceCollision"));
	TraceCollision->SetupAttachment(SceneRoot);
	TraceCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	TraceCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);
	TraceCollision->SetCollisionObjectType(ECC_GameTraceChannel4);

	RevertTraceCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RevertTraceCollision"));
	RevertTraceCollision->SetupAttachment(SceneRoot);
	RevertTraceCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RevertTraceCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	RevertTraceCollision->SetCollisionObjectType(ECC_GameTraceChannel3);

	BillboardComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComp"));
	BillboardComp->SetupAttachment(SceneRoot);


	bTraceEnabled = true;
	

}

// Called when the game starts or when spawned
void AAutoDoor_Actor::BeginPlay()
{
	Super::BeginPlay();

	TraceCollision->OnComponentBeginOverlap.AddDynamic(this, &AAutoDoor_Actor::OnOverlap);
	RevertTraceCollision->OnComponentBeginOverlap.AddDynamic(this, &AAutoDoor_Actor::OnOverlapRevert);
	
}

// Called every frame
void AAutoDoor_Actor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AAutoDoor_Actor::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UWorld* WorldRef = GetWorld();
	ASFHCharacter* CharacterRef = Cast<ASFHCharacter>(WorldRef->GetFirstPlayerController()->GetCharacter());
	if (CharacterRef)
	{
		if (OtherActor == CharacterRef)
		{
			//Open the door if not assigned to switch gravity
			if (!bGravityChange)
			{
				bDoorOpen = !bDoorOpen;
			}
		}
	}

	if (bGravityChange && GravSwitchRef)
	{
		GravSwitchRef->SwitchGravity(OtherActor);
	}


}

void AAutoDoor_Actor::OnOverlapRevert(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UWorld* WorldRef = GetWorld();
	ASFHCharacter* CharacterRef = Cast<ASFHCharacter>(WorldRef->GetFirstPlayerController()->GetCharacter());

	//Switch gravity of the actor that passed through
		if (GravSwitchRef)
		{
			GravSwitchRef->SwitchGravity(OtherActor);
		}

}