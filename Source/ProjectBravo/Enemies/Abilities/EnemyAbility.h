// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProjectBravo/Enemies/EnemyController.h"
#include "EnemyAbility.generated.h"

class AEnemyCharacter;
class APlayerCharacter;
class AEnemyAOE;

//DECLARE_DELEGATE_RetVal_OneParam(bool, FOnCanBeActivated, UEnemyAbility*);
//DECLARE_DELEGATE_OneParam(FOnActivate, UEnemyAbility*);
//DECLARE_DELEGATE_OneParam(FOnPreperationEnd, UEnemyAbility*);
//DECLARE_DELEGATE_OneParam(FOnAttackDelayEnd, UEnemyAbility*);
DECLARE_DELEGATE_OneParam(FOnAttackEnd, UEnemyAbility*);
//DECLARE_DELEGATE_OneParam(FOnAbort, UEnemyAbility*);

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBRAVO_API UEnemyAbility : public UObject
{
    GENERATED_BODY()

public:
    virtual void PostInitProperties() override;

    virtual bool CanBeActivated();

    virtual void Activate();

    virtual bool CanTick();

    virtual void Tick(float DeltaTime);

    virtual void Abort();

    int32 GetActivationPriority();

    TEnumAsByte<EStandardMovementType> GetAbilityMovementType();

    UEnemyAbility* GetNextAbility();

    float GetEntireAbilityTime();

    /*FOnCanBeActivated OnCanBeActivated;

    FOnActivate OnActivate;

    FOnPreperationEnd OnPreperationEnd;

    FOnAttackDelayEnd OnAttackDelayEnd;*/

    FOnAttackEnd OnAttackEnd;

    //FOnAbort OnAbort;

protected:
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"))
        int32 ActivationPriority = 1;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ForceUnits = "cm"))
        float MaxActivationDistance = 100;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ForceUnits = "cm"))
        float MinActivationDistance = 0;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ForceUnits = "s"))
        float PreperationTime = 1;

    UPROPERTY(EditDefaultsOnly)
        UAnimMontage* PreperationMontage;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ForceUnits = "s"))
        float AttackDelay = 1;

    UPROPERTY(EditDefaultsOnly)
        UAnimMontage* AttackDelayMontage;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float AttackTime = 1;

    UPROPERTY(EditDefaultsOnly)
        UAnimMontage* AttackMontage;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ForceUnits = "s"))
        float Cooldown = 1;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ForceUnits = "s"))
        float FirstCooldown = 0;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0001", ForceUnits = "dmg"))
        float Damage = 1;

    UPROPERTY(EditDefaultsOnly)
        TEnumAsByte<EStandardMovementType> AbilityMovementType = EStandardMovementType::StopMovement;

    UPROPERTY(EditDefaultsOnly)
        bool bCreateAOE = false;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bCreateAOE", EditConditionHides))
        TSubclassOf<AEnemyAOE> AOEClass;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bCreateAOE", EditConditionHides))
        TEnumAsByte<ECollisionChannel> AOECollisionCheckTrace;

    UPROPERTY(EditDefaultsOnly)
        bool bSetFocusInDelayAttackAndAttack = false;

    /*UPROPERTY(EditDefaultsOnly)
        bool bCanTick = false;*/

    UPROPERTY(EditDefaultsOnly)
        TSubclassOf<UEnemyAbility> NextAbilityClass;

    UPROPERTY(VisibleInstanceOnly)
        UEnemyAbility* NextAbility;

    UPROPERTY(VisibleInstanceOnly)
        bool bIsCurrentlyActive = false;

    UPROPERTY(VisibleInstanceOnly)
        AEnemyController* EnemyController;

    UPROPERTY(VisibleInstanceOnly)
        AEnemyCharacter* EnemyCharacter;

    UPROPERTY(VisibleInstanceOnly)
        APlayerCharacter* PlayerCharacter;

    FTimerHandle CooldownTimer;

    FTimerHandle PreparationTimer;

    FTimerHandle AttackDelayTimer;

    FTimerHandle AttackTimer;

    virtual void FirstCooldownEnd();

    virtual void CooldownEnd();

    virtual void PreperationEnd();

    virtual void AttackDelayEnd();

    virtual void AttackEnd();

    virtual bool TryToSpawnAOE(FVector ZPlaneOrigin, AEnemyAOE*& OutAOE);

    FORCEINLINE virtual bool TryToSpawnAOE(FVector ZPlaneOrigin);

private:
    void DrawAbilityDebugBox(FColor BoxColor, float DrawTime);
};
