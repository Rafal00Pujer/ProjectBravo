// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAOE.h"
#include "Components/SphereComponent.h"

// Sets default values
AEnemyAOE::AEnemyAOE()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    AOECollision = CreateDefaultSubobject<USphereComponent>(TEXT("AOECollision"));
    SetRootComponent(AOECollision);

    AOEDamageOverlap = CreateDefaultSubobject<USphereComponent>(TEXT("AOEDamageOverlap"));
    AOEDamageOverlap->SetupAttachment(AOECollision);
}

// Called when the game starts or when spawned
void AEnemyAOE::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AEnemyAOE::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    DrawDebugSolidBox(GetWorld(), AOECollision->GetComponentLocation(), FVector(AOECollision->GetScaledSphereRadius() * 0.8f), FColor::Green, false, DeltaTime * 2.0f);
}

const USphereComponent* AEnemyAOE::GetAOECollision() const
{
    return AOECollision;
}

