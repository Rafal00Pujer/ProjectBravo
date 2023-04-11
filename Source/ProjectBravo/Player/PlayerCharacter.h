// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "ProjectBravo/PlayerWidget.h"
#include "GameplayTagContainer.h"
#include "Weapons/PlayerWeapon.h"
#include "PlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class PROJECTBRAVO_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    APlayerCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    UPROPERTY(VisibleAnywhere)
        UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere)
        USkeletalMeshComponent* ArmsMesh;

    UPROPERTY(VisibleAnywhere)
        USkeletalMeshComponent* WeaponMesh;

    FORCEINLINE void SetVisibilityOnMeshes(bool NewVisibility);

public:
    FORCEINLINE UCameraComponent* GetCameraComponent();

#pragma region Inputs
private:
    UPROPERTY(EditDefaultsOnly, Category = "Inputs")
        UInputAction* MoveInput;

    UPROPERTY(VisibleInstanceOnly, Category = "Inputs")
        int32 MoveInputHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Inputs")
        UInputAction* LookInput;

    UPROPERTY(VisibleInstanceOnly, Category = "Inputs")
        int32 LookInputHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Inputs")
        UInputAction* JumpInput;

    UPROPERTY(VisibleInstanceOnly, Category = "Inputs")
        int32 JumpInputHandle;

    UPROPERTY(VisibleInstanceOnly, Category = "Inputs")
        int32 StopJumpInputHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Inputs")
        UInputAction* DashInput;

    UPROPERTY(VisibleInstanceOnly, Category = "Inputs")
        int32 DashInputHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Inputs")
        UInputAction* FireInput;

    UPROPERTY(VisibleInstanceOnly, Category = "Inputs")
        int32 FireInputHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Inputs")
        UInputAction* ReloadInput;

    UPROPERTY(VisibleInstanceOnly, Category = "Inputs")
        int32 ReloadInputHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Inputs")
        UInputAction* WeaponChangeInput;

    UPROPERTY(VisibleInstanceOnly, Category = "Inputs")
        int32 WeaponChangeInputHandle;

    /*UPROPERTY(EditDefaultsOnly, Category = "Inputs")
        UInputAction* RevolverInput;

    UPROPERTY(VisibleInstanceOnly, Category = "Inputs")
        int32 RevolverInputHandle;*/

    UPROPERTY(EditDefaultsOnly, Category = "Inputs")
        UInputMappingContext* PlayerCharacterGameplayMapping;

    void RemoveWeaponChangeInputs();

    void BindWeaponChangeInputs();

    void RemoveMoveAndDashInputs();

    void BindMoveAndDashInputs();

    void RemoveWeaponInputs();

    void BindWeaponInputs();
#pragma endregion

#pragma region Movement
public:
    void Immobilize(float ImmobilizeTime);

private:
    void Move(const FInputActionValue& Value);

    void Look(const FInputActionValue& Value);

    void ImmobilizeEnd();

    FTimerHandle ImmobilizeTimer;
#pragma endregion

#pragma region Dash
private:
    UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0"))
        int32 MaxDashCharges = 1;

    UPROPERTY(VisibleInstanceOnly, Category = "Dash")
        int32 CurrentDashCharges;

    UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float DashChargeCooldown = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0.0", ForceUnits = "cm"))
        float DashDistance = 100;

    UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float DashTime = 1;

    UPROPERTY(VisibleInstanceOnly, Category = "Dash", meta = (ForceUnits = "cm/s"))
        /// <summary>
        /// Calculated dash speed.
        /// </summary>
        float DashSpeed;

    UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0.0", ClampMax = "1", ForceUnits = "%"))
        /// <summary>
        /// Used to stop the dash when colliding with something.
        /// </summary>
        float MinDashSpeedPercent = 0.5;

    UPROPERTY(VisibleInstanceOnly, Category = "Dash", meta = (ForceUnits = "cm/s"))
        /// <summary>
        /// Calculated min dash speed.
        /// </summary>
        float MinDashSpeed;

    UPROPERTY(VisibleInstanceOnly, Category = "Dash")
        /// <summary>
        /// Used to store braking friction factor.
        /// </summary>
        float BrakingFrictionFactorSave;

    UPROPERTY(VisibleInstanceOnly, Category = "Dash")
        /// <summary>
        /// Used to store braking deceleration walking.
        /// </summary>
        float BrakingDecelerationWalkingSave;

    FTimerHandle DashTimer;

    FTimerHandle AddDashChargeTimer;

    void StartDash();

    FORCEINLINE void TickDash();

    void EndDash();

    void AddDashCharge();
#pragma endregion

#pragma region Health
public:
    void AddPhysicalDamage(float DamageAmount, FVector HitDirectionInWorld);

    void Heal(float HealthAmount, float ArmorAmount);

    FORCEINLINE bool HasCriticalHealth();

    float GetCurrentHealth();

    float GetMaxHealth();

    float GetCurrentArmor();

    float GetMaxArmor();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1"))
        float MaxHealth = 100;

    UPROPERTY(VisibleInstanceOnly, Category = "Health")
        float CurrentHealth;

    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1"))
        float MaxArmor = 100;

    UPROPERTY(VisibleInstanceOnly, Category = "Health")
        float CurrentArmor;

    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "1", ForceUnits = "%"))
        float CriticalHealthActivation = 0.5;

    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "1", ForceUnits = "%"))
        float CriticalHealthDamageReduction = 0.5;

    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "1", ForceUnits = "%"))
        float DamageReductionFromBack = 0.5;

    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "1", ForceUnits = "%"))
        float DamageToHealth = 0.5;

    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "1", ForceUnits = "%"))
        float DamageToArmor = 0.5;

    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "1", ForceUnits = "%"))
        float DamageReductionWithArmor = 0.5;

    UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "0.0", ForceUnits = "%"))
        float HealingIncreaseOnCriticalHealth = 0.5;
#pragma endregion

#pragma region Weapons
public:
    FORCEINLINE void PlayArmsMontage(UAnimMontage* Montage, float Speed);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="WeaponFName"></param>
    /// <param name="AmmoAmount">Ammo amount to add. If the added amount will result the pocket to overflow, then this param will return that overflow amount.</param>
    void AddAmmo(FGameplayTag WeaponTag, int32 AmmoAmount);

    void AddWeapon(FGameplayTag WeaponTag);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Weapons")
        TArray<TSubclassOf<UPlayerWeapon>> WeaponsClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapons")
        /// <summary>
        /// First index in array will be the selected weapon.
        /// </summary>
        TArray<int32> StartingWeapons;

    UPROPERTY(EditDefaultsOnly, Category = "Weapons")
        bool bAutoWeaponPickUp = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
        TArray<UPlayerWeapon*> PlayerWeapons;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
        UPlayerWeapon* SelectedWeapon = nullptr;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
        UMaterialInstanceDynamic* WeaponMaterial;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
        /// <summary>
        /// Set to -1 for no next weapon.
        /// </summary>
        int32 NextWeaponIndex = -1;

    FTimerHandle WeaponChangeTimer;

    void UpdateWeaponColor();

    void Fire();

    void Reload();

    void WeaponChangeInputFunc(const FInputActionValue& Value);

    void WeaponChange(int32 WeaponIndex);

    void StartChangeOutWeapon();

    void EndChangeOutWeapon();

    void StartChangeInWeapon();

    void EndChangeInWeapon();

#pragma endregion

#pragma region UI
public:
    void UpdateAmmoUI();

private:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
        TSubclassOf<UPlayerWidget> PlayerWidgetClass;

    UPROPERTY(VisibleInstanceOnly, Category = "UI")
        UPlayerWidget* PlayerWidget;

    void CreateWidgetAndUpdateUI();

    void UpdateHealtAndArmorUI();

    void UpdateDashUI();

    void UpdateWeaponNameAndColorUI();
#pragma endregion

#pragma region Console Comands
private:
    UFUNCTION(Exec)
        void AddPhysicalDamageConsole(float DamageAmount);

    UFUNCTION(Exec)
        void HealConsole(float HealthAmount, float ArmorAmount);

    UFUNCTION(Exec)
        void AddAmmoConsole(FGameplayTag WeaponTag, int32 AmmoAmount);

    UFUNCTION(Exec)
        void AddWeaponConsole(FGameplayTag WeaponTag);

    UFUNCTION(Exec)
        void AddAllWeaponsConsole();

    UFUNCTION(Exec)
        void RemoveWeaponConsole(FGameplayTag WeaponTag);

    UFUNCTION(Exec)
        void RemoveAllWeaponsConsole();

    UFUNCTION(Exec)
        void HideCurrentWeaponConsole();
#pragma endregion
};