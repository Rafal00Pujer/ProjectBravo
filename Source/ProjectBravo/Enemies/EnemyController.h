// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyCharacter.h"
#include "EnemyController.generated.h"

class UEnemyAbility;
class UProjectileEnemyAbility;

UENUM()
enum EStandardMovementType
{
    StopMovement,
    RandomAroundPlayer,
    RandomOnLevel,
    MoveToPlayer
};

/**
 *
 */
UCLASS()
class PROJECTBRAVO_API AEnemyController : public AAIController
{
    GENERATED_BODY()

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Override orginal function, because it sets Pitch to 0.
    virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn) override;

    virtual void Destroyed() override;

private:
    AEnemyCharacter* EnemyCharacter;

    virtual void OnEnemyAddDamage(FGameplayTag GrenadeTag, FGameplayTag WeaponTag, float DamageScale, float Damage);

#pragma region Abilities
public:
    void SetCanActivateAbilities(bool NewCanActivateAbilities);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
        TArray<TSubclassOf<UEnemyAbility>> AbilitiesClass;

    UPROPERTY(VisibleInstanceOnly, Category = "Abilities")
        TArray<UEnemyAbility*> Abilities;

    UPROPERTY(VisibleInstanceOnly, Category = "Abilities")
        UEnemyAbility* CurrentAbility;

    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
        bool bCanActivateAbilities;

    UPROPERTY(EditDefaultsOnly, Category = "Abilities", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float SpawnAbilityActivationDelay = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Abilities", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float DelayForNextAbilityActivationTry = 1;

    FTimerHandle NextAbilityActivationTryTimer;

    void ActivateNewAbility();

    void ActivateCurrentAbility();

    void OnAbilityAttackEnd(UEnemyAbility* Ability);
#pragma endregion

#pragma region Movement
private:
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
        TEnumAsByte<EStandardMovementType> DefaultMovementType = EStandardMovementType::StopMovement;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement")
        TEnumAsByte<EStandardMovementType> CurrentMovementType;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.0", ForceUnits = "cm"))
        float RandomAroundPlayerRadius = 100;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.0", ForceUnits = "cm"))
        float RandomOnLevelRadius = 100;

    UPROPERTY(VisibleInstanceOnly, Category = "Movement")
        AEnemyCharacter* MoveToEnemyTarget;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "-1.0", ForceUnits = "cm"))
        float MoveToPlayerAcceptanceRadius = 150;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "-1.0", ForceUnits = "cm"))
        float MoveToEnemyAcceptanceRadius = 150;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "1", ForceUnits = "s"))
        float DelayForNextMovementRequestOnFail = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "1", ForceUnits = "s"))
        float DelayForNextMovementRequest = 1;

    FTimerHandle DelayForNextMovementRequestOnFailTimer;

    FTimerHandle DelayForNextMovementRequestTimer;

    void HandleMovement();

    void OnMovementRequestFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);

#pragma endregion

#pragma region Dodge
public:
    bool IsDodging();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Dodge")
        bool bCanDodge = false;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides))
        FName DodgeCollisionPreset;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides))
        float EnemyCapsuleInflation = -3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides, ClampMin = "0", ForceUnits = "s"))
        float DodgeFirstCooldown = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides, ClampMin = "0", ForceUnits = "s"))
        float DodgeCooldown = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides, ClampMin = "0.0001", ForceUnits = "s"))
        float DodgeTime = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides, ClampMin = "10", ForceUnits = "cm"))
        float DodgeDistance = 10;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides, ClampMin = "10", ForceUnits = "cm"))
        float MaxDodgeInPlaceDistance = 10;

    UPROPERTY(VisibleInstanceOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides, ForceUnits = "cm/s"))
        float DodgeSpeed;

    UPROPERTY(VisibleInstanceOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides))
        float BrakingFrictionFactorSave;

    UPROPERTY(VisibleInstanceOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides))
        float BrakingDecelerationWalkingSave;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides))
        UAnimMontage* DodgeLeftMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides))
        UAnimMontage* DodgeRightMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides))
        UAnimMontage* DodgeNoMoveMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides))
        TSubclassOf<UProjectileEnemyAbility> DodgeParallelAbilityClass;

    UPROPERTY(VisibleInstanceOnly, Category = "Dodge", meta = (EditCondition = "bCanDodge", EditConditionHides))
        UEnemyAbility* DodgeParallelAbility;

    FTimerHandle DodgeTimer;

    FTimerHandle DodgeCooldownTimer;

    void DodgeStart();

    void DodgeEnd();

    /*bool IsDodgePathClear(float Direction, FVector RightVector);*/

    float GetDirectionClearDistance(float Direction, FVector RightVector);
#pragma endregion

};
