// Fill out your copyright notice in the Description page of Project Settings.

#include "LiftVolume.h"
#include "DragonLocomotionCharacter.h"

// Sets default values
ALiftVolume::ALiftVolume()
{
 	// Set this actor to call Tick() every frame. 
	PrimaryActorTick.bCanEverTick = false;

	LiftVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("LiftVolume"));
	RootComponent = LiftVolume;

	LiftVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LiftVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	LiftVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	LiftVolume->OnComponentBeginOverlap.AddDynamic(
		this,
		&ALiftVolume::OnBeginOverlap
	);

	LiftVolume->OnComponentEndOverlap.AddDynamic(
		this,
		&ALiftVolume::OnEndOverlap
	);
}

// Called when the game starts or when spawned
void ALiftVolume::BeginPlay()
{
	Super::BeginPlay();

}

void ALiftVolume::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ADragonLocomotionCharacter* Dragon =
		Cast<ADragonLocomotionCharacter>(OtherActor);

	if (!Dragon)
	{
		return;
	}

	// Dragon has entered the lift
	Dragon->SetLiftAcceleration(LiftStrength);
}

void ALiftVolume::OnEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ADragonLocomotionCharacter* Dragon =
		Cast<ADragonLocomotionCharacter>(OtherActor);

	if (!Dragon)
	{
		return;
	}

	// Dragon has left the lift
	Dragon->SetLiftAcceleration(0.0f);
}


