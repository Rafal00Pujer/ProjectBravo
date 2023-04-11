// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include <ProjectBravo/Projectile.h>
#include <ProjectBravo/PickUp.h>
#include "PlayerWeapon.generated.h"

UENUM()
enum EPlayerBulletType
{
    Standard,
    Ricochet,
    Grenade
};

USTRUCT()
struct FBulletInfo
{
    GENERATED_BODY();

public:
    TSubclassOf<AProjectile> GetProjectileClass() const;

    float GetOffsetFromCenter() const;

    float GetTraceDistance() const;

    const FRotator& GetMinRotationOffset() const;

    const FRotator& GetMaxRotationOffset() const;

    float GetDamageScale() const;

    float GetTacticalDamageScale() const;

    TEnumAsByte<EPlayerBulletType> GetBulletType() const;

    float GetRicochetChance() const;

    int32 GetNumOfRicochets() const;

    float GetRicochetMaxDistance() const;

    float GetRicochetDamageScaleMultiply() const;

    float GetGrenadeRadius() const;

    float GetGrenadePlayerSelfDamage() const;

    FGameplayTag GetGrenadeTag() const;

private:
    UPROPERTY(EditDefaultsOnly)
        TSubclassOf<AProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, meta = (ForceUnits = "cm"))
        float OffsetFromCenter = 0;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1.0", ForceUnits = "cm"))
        float TraceDistance = 1;

    UPROPERTY(EditDefaultsOnly)
        FRotator MinRotationOffset;

    UPROPERTY(EditDefaultsOnly)
        FRotator MaxRotationOffset;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0001"))
        float DamageScale = 1;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0001"))
        float TacticalDamageScale = 1;

    UPROPERTY(EditDefaultsOnly)
        TEnumAsByte<EPlayerBulletType> BulletType = Standard;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "BulletType == EPlayerBulletType::Ricochet", EditConditionHides, ClampMin = "0.0001", ClampMax = "1", ForceUnits = "%"))
        float RicochetChance = 1;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "BulletType == EPlayerBulletType::Ricochet", EditConditionHides, ClampMin = "1"))
        int32 NumOfRicochets = 1;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "BulletType == EPlayerBulletType::Ricochet", EditConditionHides, ClampMin = "0.0001", ForceUnits = "cm"))
        float RicochetMaxDistance = 1;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "BulletType == EPlayerBulletType::Ricochet", EditConditionHides))
        float RicochetDamageScaleMultiply = 1;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "BulletType == EPlayerBulletType::Grenade", EditConditionHides, ClampMin = "10.0", ForceUnits = "cm"))
        float GrenadeRadius = 1;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "BulletType == EPlayerBulletType::Grenade", EditConditionHides, ClampMin = "0"))
        float GrenadePlayerSelfDamage = 0;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "BulletType == EPlayerBulletType::Grenade", EditConditionHides, ClampMin = "10.0", ForceUnits = "cm"))
        FGameplayTag GrenadeTag;
};

USTRUCT()
struct FBulletsPattern
{
    GENERATED_BODY();

public:
    const TArray<FBulletInfo>& GetBullets() const;

    float GetAttackTime() const;

    UAnimMontage* GetAttackMontage() const;

    float GetAttackMontageSpeed() const;

private:
    UPROPERTY(EditDefaultsOnly)
        TArray<FBulletInfo> Bullets;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float AttackTime = 1;

    UPROPERTY(EditDefaultsOnly)
        UAnimMontage* AttackMontage;

    UPROPERTY(EditDefaultsOnly, meta = (InlineEditConditionToggle))
        bool bAttackMontageTimeOverride = false;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bAttackMontageTimeOverride", ForceUnits = "s"))
        float AttackMontageTimeOverride = 1;
};

USTRUCT()
struct FWeaponSeries
{
    GENERATED_BODY()

public:
    const TArray<FBulletsPattern>& GetBulletPatterns() const;

    const TArray<TSubclassOf<APickUp>>& GetPickUpsClass() const;

    float GetPreperationTime() const;

    UAnimMontage* GetPreperationMontage() const;

    float GetPreperationMontageSpeed() const;

    float GetRecoveryTime() const;

    UAnimMontage* GetRecoveryMontage() const;

    float GetRecoveryMontageSpeed() const;

private:
    UPROPERTY(EditDefaultsOnly)
        TArray<FBulletsPattern> BulletsPatterns;

    UPROPERTY(EditDefaultsOnly)
        TArray<TSubclassOf<APickUp>> PickUpsClass;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ForceUnits = "s"))
        float PreperationTime = 1;

    UPROPERTY(EditDefaultsOnly)
        UAnimMontage* PreperationMontage;

    UPROPERTY(EditDefaultsOnly, meta = (InlineEditConditionToggle))
        bool bPreperationMontageTimeOverride = false;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bPreperationMontageTimeOverride", ForceUnits = "s"))
        float PreperationMontageTimeOverride = 1;

    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ForceUnits = "s"))
        float RecoveryTime = 1;

    UPROPERTY(EditDefaultsOnly)
        UAnimMontage* RecoveryMontage;

    UPROPERTY(EditDefaultsOnly, meta = (InlineEditConditionToggle))
        bool bRecoveryMontageTimeOverride = false;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bRecoveryMontageTimeOverride", ForceUnits = "s"))
        float RecoveryMontageTimeOverride = 1;
};

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBRAVO_API UPlayerWeapon : public UObject
{
    GENERATED_BODY()

public:
    virtual void PostInitProperties() override;

    FText GetWeaponName();

    FGameplayTag GetWeaponTag();

    FLinearColor GetWeaponColor();

    bool GetIsHidden();

    void SetIsHidden(bool IsHidden);

    bool GetIsSelected();

    void SetIsSelected(bool IsSelected);

    static float CalculateMontageSpeed(UAnimMontage* Montage, bool OverrideTime, float TimeOverrideVale);

private:
    UPROPERTY(EditDefaultsOnly)
        FText WeaponName;

    UPROPERTY(EditDefaultsOnly)
        FGameplayTag WeaponTag;

    UPROPERTY(EditDefaultsOnly)
        FLinearColor Color;

    UPROPERTY(VisibleInstanceOnly)
        bool bIsHidden = true;

    UPROPERTY(VisibleInstanceOnly)
        bool bIsSelected = false;

#pragma region Ammo
public:
    bool CanBeReloaded();

    void Reload();

    void AddAmmo(int32 AmmoAmount);

    int32 GetCurrentClipAmmo();

    int32 GetMaxClipAmmo();

    int32 GetCurrentPocketAmmo();

    int32 GetMaxPocketAmmo();

    bool GetIsReloading();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (ClampMin = "1"))
        int32 MaxClipAmmo = 1;

    UPROPERTY(VisibleInstanceOnly, Category = "Ammo")
        int32 CurrentClipAmmo = 0;

    UPROPERTY(VisibleInstanceOnly, Category = "Ammo")
        int32 TacticalAmmo = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (ClampMin = "1"))
        int32 MaxPocketAmmo = 1;

    UPROPERTY(VisibleInstanceOnly, Category = "Ammo")
        int32 CurrentPocketAmmo = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float EmptyReloadTime = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float TacticalReloadTime = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float PocketReloadAdditionalTime = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo")
        UAnimMontage* EmptyReloadMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (InlineEditConditionToggle))
        bool bEmptyReloadMontageTimeOverride = false;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (EditCondition = "bEmptyReloadMontageTimeOverride", ForceUnits = "s"))
        float EmptyReloadMontageTimeOverride = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo")
        UAnimMontage* TacticalReloadMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (InlineEditConditionToggle))
        bool bTacticalReloadMontageTimeOverride = false;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (EditCondition = "bTacticalReloadMontageTimeOverride", ForceUnits = "s"))
        float TacticalReloadMontageTimeOverride = 1;

    FTimerHandle ReloadTimer;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo")
        bool bCanRegenerateAmmo = false;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (EditCondition = "bCanRegenerateAmmo", EditConditionHides, ClampMin = "0"))
        int32 AmmoRegenerationTreshold = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (EditCondition = "bCanRegenerateAmmo", EditConditionHides, ClampMin = "1"))
        int32 AmmoRegenerationAmount = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (EditCondition = "bCanRegenerateAmmo", EditConditionHides, ClampMin = "0.0001", ForceUnits = "s"))
        float AmmoRegenerationTime = 1;

    FTimerHandle AmmoRegenerationTimer;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo")
        bool bCanThrowClip = false;

    UPROPERTY(EditDefaultsOnly, Category = "Ammo", meta = (EditCondition = "bCanThrowClip", EditConditionHides))
        FWeaponSeries ClipSeries;

    UPROPERTY(VisibleInstanceOnly, Category = "Ammo", meta = (EditCondition = "bCanThrowClip", EditConditionHides))
        bool bIsThrowingClip = false;

    void ReloadEnd();

#pragma endregion

#pragma region Attack
public:
    bool CanFire();

    void Fire();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Attack")
        TArray<FWeaponSeries> ShootingPattern;

    UPROPERTY(VisibleInstanceOnly, Category = "Attack")
        int32 CurrentShotIndex = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Attack")
        bool bResetAttackPatternOnReload = false;

    UPROPERTY(EditDefaultsOnly, Category = "Attack")
        bool bResetAttackPatternOnChangeIn = false;

    /*UPROPERTY(VisibleInstanceOnly, Category = "Attack")*/
    FWeaponSeries* CurrentShot;

    UPROPERTY(VisibleInstanceOnly, Category = "Attack")
        int32 CurrentBulletsPatternIndex = 0;

    /*UPROPERTY(VisibleInstanceOnly, Category = "Attack")*/
    const FBulletsPattern* CurrentBulletsPattern;

    FTimerHandle PreperationTimer;

    FTimerHandle AttackTimer;

    FTimerHandle RecoveryTimer;

    void PreperationStart(bool FromClipThrow);

    void PreperationEnd(bool FromClipThrow);

    void Attack(bool FromTacticalAmmo, bool FromClipThrow);

    void DealDamage(const AProjectile* Projectile, const FHitResult& Hit, const FBulletInfo& BulletInfo, const TArray<TSubclassOf<APickUp>>& PickUpsClass, float DamageScale);

#pragma endregion

#pragma region Shield
public:
    bool IsShieldActive();

    /// <summary>
    /// 
    /// </summary>
    /// <param name="Damage"></param>
    /// <returns>Unabsorbed damage.</returns>
    float AbsorbDamage(float DamageToAbsorb);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Shield")
        bool bHasShield = false;

    UPROPERTY(EditDefaultsOnly, Category = "Shield", meta = (EditCondition = "bHasShield", EditConditionHides, ClampMin = "0.0001", ForceUnits = "%"))
        float DamageAbsorption = 0.5;

    UPROPERTY(EditDefaultsOnly, Category = "Shield", meta = (EditCondition = "bHasShield", EditConditionHides, ClampMin = "0.0001"))
        float MaxAbsorbedDamage = 100;

    UPROPERTY(EditDefaultsOnly, Category = "Shield", meta = (EditCondition = "bHasShield", EditConditionHides, ClampMin = "1"))
        int32 RemoveAmmoAmount = 1;

    UPROPERTY(VisibleInstanceOnly, Category = "Shield")
        float CurrentAbsorbedDamage = 0;

#pragma endregion

#pragma region Weapon Change
public:
    bool CanBeChangedIn();

    TTuple<float, UAnimMontage*, float> ChangeIn();

    bool CanBeChangedOut();

    TTuple<float, UAnimMontage*, float> ChangeOut();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon Change", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float ChangeInTime = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Change")
        UAnimMontage* ChangeInMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Change", meta = (InlineEditConditionToggle))
        bool bChangeInMontageTimeOverride = false;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Change", meta = (EditCondition = "bChangeInMontageTimeOverride", ForceUnits = "s"))
        float ChangeInMontageTimeOverride = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Change", Category = "Weapon Change", meta = (ClampMin = "0.0001", ForceUnits = "s"))
        float ChangeOutTime = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Change")
        UAnimMontage* ChangeOutMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Change", meta = (InlineEditConditionToggle))
        bool bChangeOutMontageTimeOverride = false;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Change", meta = (EditCondition = "bChangeOutMontageTimeOverride", ForceUnits = "s"))
        float ChangeOutMontageTimeOverride = 1;

#pragma endregion

};
