// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorPanel_Actor.h"
#include "Containers/UnrealString.h"
#include "SFHCharacter.h"

// Sets default values
ADoorPanel_Actor::ADoorPanel_Actor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	PanelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanelMesh"));
	PanelMesh->SetupAttachment(RootComponent);

	TraceCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("TraceCollision"));
	TraceCollision->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADoorPanel_Actor::BeginPlay()
{
	Super::BeginPlay();

	TraceCollision->OnComponentBeginOverlap.AddDynamic(this, &ADoorPanel_Actor::OnOverlap);
	TraceCollision->OnComponentEndOverlap.AddDynamic(this, &ADoorPanel_Actor::OnOverlapEnd);
	
}

// Called every frame
void ADoorPanel_Actor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ADoorPanel_Actor::RequestInteraction_Implementation()
{
	if (!bIsSwitchEnabled)
	{
		bIsSwitchEnabled = true;
	}

	else
	{
		bIsSwitchEnabled = false;
	}
	
	return true;
}

bool ADoorPanel_Actor::RequestHover_Implementation()
{
	return true;
}


void ADoorPanel_Actor::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UWorld* WorldRef = GetWorld();
	ASFHCharacter* CharacterRef = Cast<ASFHCharacter>(WorldRef->GetFirstPlayerController()->GetCharacter());
	if (CharacterRef)
	{
		if (OtherActor == CharacterRef)
		{
			CharacterRef->bEnableHoverCheck = true;
			CharacterRef->OverlapCount += 1;
		}

	}
}

void ADoorPanel_Actor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UWorld* WorldRef = GetWorld();
	ASFHCharacter* CharacterRef = Cast<ASFHCharacter>(WorldRef->GetFirstPlayerController()->GetCharacter());
	if (CharacterRef)
	{
		if (OtherActor == CharacterRef)
		{

			CharacterRef->OverlapCount -= 1;
#
			if (CharacterRef->OverlapCount == 0)
			{
				CharacterRef->bEnableHoverCheck = false;
			}

			if (CharacterRef->WidgetRef != nullptr)
			{
				CharacterRef->RemoveIconWidget();
			}
		}

	}
}

