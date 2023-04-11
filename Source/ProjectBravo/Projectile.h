// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Projectile.generated.h"

DECLARE_DELEGATE_TwoParams(FOnProjectileStopAndDestroy, const AProjectile*, const FHitResult&)

UCLASS()
class PROJECTBRAVO_API AProjectile : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AProjectile();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    FORCEINLINE float GetSphereRadius() const;

    FORCEINLINE FName GetCollisionProfile() const;

    FORCEINLINE UParticleSystem* GetHitParticle() const;

    UProjectileMovementComponent* GetProjectileMovementComponent() const;

    FOnProjectileStopAndDestroy OnProjectileStopAndDestroy;

protected:
    UPROPERTY(VisibleAnywhere)
        USphereComponent* SphereCollision;

    UPROPERTY(VisibleAnywhere)
        UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(VisibleAnywhere)
        UParticleSystemComponent* PartilceComponent;

    UPROPERTY(EditDefaultsOnly)
        UParticleSystem* HitParticle;

    UFUNCTION()
        void OnProjectileStop(const FHitResult& ImpactResult);
};
