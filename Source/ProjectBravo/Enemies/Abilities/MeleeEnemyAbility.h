// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyAbility.h"
#include "MeleeEnemyAbility.generated.h"

class USplineComponent;

/**
 *
 */
UCLASS()
class PROJECTBRAVO_API UMeleeEnemyAbility : public UEnemyAbility
{
    GENERATED_BODY()

public:
    virtual void PostInitProperties() override;

    virtual void Activate() override;

    virtual bool CanTick() override;

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void AttackDelayEnd() override;

    virtual void AttackEnd() override;

private:
    UPROPERTY(EditDefaultsOnly)
        FName MeleeSplineComponentName;

    UPROPERTY(EditDefaultsOnly)
        FName MeleeTraceProfile;

    UPROPERTY(VisibleInstanceOnly)
        USplineComponent* MeleeSplineComponent;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ForceUnits = "cm"))
        float MelleSphereRadius = 10;

    UPROPERTY(EditDefaultsOnly, meta = (InlineEditConditionToggle))
        bool bCanImmobilize = false;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0001", ForceUnits = "s", EditCondition = "bCanImmobilize"))
        float ImmobilizeTime = 1;

    UPROPERTY(VisibleInstanceOnly)
        float CurrentSplineTime = 0.0f;

    UPROPERTY(VisibleInstanceOnly)
        bool bPlayerIsHit = false;

    UPROPERTY(VisibleInstanceOnly)
        bool bStartSplineInterp = false;

    /*UPROPERTY(VisibleInstanceOnly)*/
        FCollisionShape MeleeCollision;
};
