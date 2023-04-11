// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyAbility.h"
#include "DashEnemyAbility.generated.h"

/**
 *
 */
UCLASS()
class PROJECTBRAVO_API UDashEnemyAbility : public UEnemyAbility
{
    GENERATED_BODY()

public:
    virtual void PostInitProperties() override;

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    virtual bool CanBeActivated() override;

    virtual void Activate() override;

    virtual bool CanTick() override;

    virtual void Tick(float DeltaTime) override;

    virtual void Abort() override;

protected:
    virtual void AttackDelayEnd() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Dodge")
        FName DodgeCollisionPreset;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge")
        float EnemyCapsuleInflation = -3.0f;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "10", ForceUnits = "cm"))
        float MaxDistanceDifferenceForPathCheck = 10;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "10", ForceUnits = "cm"))
        float DashDistance = 10;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0001", ForceUnits = "s"))
        /// <summary>
        /// If the Dash time will be lower than the attack time, then the enemy will be stuned for remaining attack time, if enemy didn't hit the player.
        /// </summary>
        float DashTime = 1;

    UPROPERTY(VisibleInstanceOnly, meta = (ForceUnits = "s"))
        float StunTime = 0;

    UPROPERTY(VisibleInstanceOnly, meta = (ForceUnits = "cm/s"))
        float DashSpeed = 10;

    UPROPERTY(VisibleInstanceOnly)
        float BrakingFrictionFactorSave;

    UPROPERTY(VisibleInstanceOnly)
        float BrakingDecelerationWalkingSave;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1", ForceUnits = "cm"))
        float TraceDistance = 10;

    FTimerHandle DashTimer;

    void DashEnd();
};
