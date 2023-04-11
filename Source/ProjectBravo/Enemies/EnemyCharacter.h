// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "ProjectBravo/Projectile.h"
#include "ProjectBravo/PickUp.h"
#include "EnemyCharacter.generated.h"

DECLARE_DELEGATE_FourParams(FOnEnemyAddDamage, FGameplayTag, FGameplayTag, float, float);

UCLASS()
class PROJECTBRAVO_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AEnemyCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region Pick Ups Spawn
private:
    UPROPERTY(VisibleAnywhere, Category = "PickUps Spawn")
        USceneComponent* FirstPickUpSpawn;

    UPROPERTY(VisibleAnywhere, Category = "PickUps Spawn")
        USceneComponent* SecondPickUpSpawn;

    UPROPERTY(VisibleAnywhere, Category = "PickUps Spawn")
        USceneComponent* ThirdPickUpSpawn;

    UPROPERTY(VisibleAnywhere, Category = "PickUps Spawn")
        USceneComponent* FourthPickUpSpawn;

    UPROPERTY(EditDefaultsOnly, Category = "PickUps Spawn")
        FVector MinPickUpSpawnImpuls;

    UPROPERTY(EditDefaultsOnly, Category = "PickUps Spawn")
        FVector MaxPickUpSpawnImpuls;

    UPROPERTY(EditDefaultsOnly, Category = "PickUps Spawn")
        TArray<TSubclassOf<APickUp>> PlayerCriticalHealthPickUpsClass;

    void SpawnRandomPickUP(const TArray<TSubclassOf<APickUp>>& PickUpsClass);

    void SpawnPlayerCriticalHealthPickUp();
#pragma endregion

#pragma region Health
public:
    FOnEnemyAddDamage OnEnemyAddDamage;

    void AddDamage(FGameplayTag GrenadeTag, FGameplayTag WeaponTag, float DamageScale, const TArray<TSubclassOf<APickUp>>& PickUpsClass);

    float GetCurrentHealth();

    bool IsAlive();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1"))
        float MaxHealth = 1000;

    UPROPERTY(VisibleInstanceOnly, Category = "Health")
        float CurrentHealth;

    UPROPERTY(EditDefaultsOnly, Category = "Health")
        FGameplayTag StandardGrenadeTag;

    UPROPERTY(EditDefaultsOnly, Category = "Health")
        bool bIsImmuneToExplosive = false;

    UPROPERTY(EditDefaultsOnly, Category = "Health")
        TMap<FGameplayTag, float> DamageTable;

    UPROPERTY(EditDefaultsOnly, Category = "Health")
        UAnimMontage* DeathMontage;

    FTimerHandle DeathTimer;

    void AddDamage(float Damage);

    void DeathMontageEnd();

#pragma endregion

#pragma region Player Grenade Effect Corrosive
private:
    UPROPERTY(EditDefaultsOnly, Category = "Player Grenade Effect Corrosive")
        FGameplayTag CorrosiveTag;

    UPROPERTY(EditDefaultsOnly, Category = "Player Grenade Effect Corrosive", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float CorrosiveGrenadeTime = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Player Grenade Effect Corrosive", meta = (ClampMin = "1.0"))
        float CorrosiveDamageMultiply = 1;

    FTimerHandle CorrosiveTimer;

    void StartCorrosiveGrenade();

#pragma endregion

#pragma region Player Grenade Effect Electric
private:
    UPROPERTY(EditDefaultsOnly, Category = "Player Grenade Effect Electric")
        FGameplayTag ElectricTag;

    UPROPERTY(EditDefaultsOnly, Category = "Player Grenade Effect Electric", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float ElectricGrenadeTime = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Player Grenade Effect Electric", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
        float MovementSpeedWhileOnEffect = 300.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Player Grenade Effect Electric", meta = (ForceUnits = "cm/s"))
        float MovementSpeedSave;

    FTimerHandle ElectricTimer;

    void StartElectricGrenade();

    void EndElectricGrenade();

#pragma endregion

#pragma region Player Grenade Effect Incendary
private:
    UPROPERTY(EditDefaultsOnly, Category = "Player Grenade Effect Incendary")
        FGameplayTag IncendaryTag;

    UPROPERTY(EditDefaultsOnly, Category = "Player Grenade Effect Incendary", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float IncendaryGrenadeTime = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Player Grenade Effect Incendary", meta = (ClampMin = "0.0001", ForceUnits = "dmg/s"))
        float IncendaryDamage = 1;

    FTimerHandle IncendaryTimer;

    void StartIncendaryGrenade();

    void IncendaryGrenadeTick(float DeltaTime);

#pragma endregion
};
