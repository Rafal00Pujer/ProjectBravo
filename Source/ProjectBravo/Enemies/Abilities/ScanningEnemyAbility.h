// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyAbility.h"
#include "ScanningEnemyAbility.generated.h"

/**
 *
 */
UCLASS()
class PROJECTBRAVO_API UScanningEnemyAbility : public UEnemyAbility
{
    GENERATED_BODY()

public:
    virtual void PostInitProperties() override;

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void PreperationEnd() override;

    virtual void AttackDelayEnd() override;

private:
    UPROPERTY(EditDefaultsOnly)
        FName ScanningStartPointSceneComponentName;

    UPROPERTY(EditDefaultsOnly)
        TEnumAsByte<ECollisionChannel> ScanningTraceChannel;

    UPROPERTY(VisibleInstanceOnly)
        USceneComponent* ScanningStartPoint;

    UPROPERTY(VisibleInstanceOnly)
        FRotator ScanningRotationToPlayer;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ForceUnits = "cm"))
        float MaxScanningDistance = 100;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ForceUnits = "cm"))
        float ScanningSphereRadius = 10;

    void DrawAbilitDebugLine(float DeltaTime);
};