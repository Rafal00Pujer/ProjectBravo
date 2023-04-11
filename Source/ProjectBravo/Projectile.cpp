// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Enemies/EnemyCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectile::AProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);

	PartilceComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PartilceSystem"));
	PartilceComponent->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AProjectile::OnProjectileStop);
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AProjectile::GetSphereRadius() const
{
	return SphereCollision->GetScaledSphereRadius();
}

FName AProjectile::GetCollisionProfile() const
{
	return SphereCollision->GetCollisionProfileName();
}

UParticleSystem* AProjectile::GetHitParticle() const
{
	return HitParticle;
}

UProjectileMovementComponent* AProjectile::GetProjectileMovementComponent() const
{
    return ProjectileMovement;
}

void AProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("Projectile impact with: %s"), *ImpactResult.GetActor()->GetName());

	OnProjectileStopAndDestroy.ExecuteIfBound(this, ImpactResult);

	Destroy();
}

