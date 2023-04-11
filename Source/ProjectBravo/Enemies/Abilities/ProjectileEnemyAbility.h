// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyAbility.h"
#include "ProjectileEnemyAbility.generated.h"

class AProjectile;

/**
 *
 */
UCLASS()
class PROJECTBRAVO_API UProjectileEnemyAbility : public UEnemyAbility
{
    GENERATED_BODY()

public:
    virtual void PostInitProperties() override;

protected:
    virtual void PreperationEnd() override;

    virtual void AttackDelayEnd() override;

private:
    UPROPERTY(EditDefaultsOnly)
        FName ProjectileStartPointSceneComponentName;

    UPROPERTY(VisibleInstanceOnly)
        USceneComponent* ProjectileStartPoint;

    UPROPERTY(EditDefaultsOnly)
        TSubclassOf<AProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly)
        FVector ProjectileMinTargetOffset;

    UPROPERTY(EditDefaultsOnly)
        FVector ProjectileMaxTargetOffset;

    UPROPERTY(VisibleInstanceOnly)
        FVector ProjectileTargetPoint;

    UPROPERTY(EditDefaultsOnly, meta = (InlineEditConditionToggle))
        bool bIsGrenade = false;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0001", ForceUnits = "cm", EditCondition = "bIsGrenade"))
        float GrenadeRadius = 1;

    void DealDamage(const AProjectile* Projectile, const FHitResult& ImpactResult);
};
