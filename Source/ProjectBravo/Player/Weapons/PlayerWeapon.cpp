// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWeapon.h"
#include "ProjectBravo/Player/PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "ProjectBravo/Enemies/EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#pragma region Bullet Info
TSubclassOf<AProjectile> FBulletInfo::GetProjectileClass() const
{
    return ProjectileClass;
}

float FBulletInfo::GetOffsetFromCenter() const
{
    return OffsetFromCenter;
}

float FBulletInfo::GetTraceDistance() const
{
    return TraceDistance;
}

const FRotator& FBulletInfo::GetMinRotationOffset() const
{
    return MinRotationOffset;
}

const FRotator& FBulletInfo::GetMaxRotationOffset() const
{
    return MaxRotationOffset;
}

float FBulletInfo::GetDamageScale() const
{
    return DamageScale;
}

float FBulletInfo::GetTacticalDamageScale() const
{
    return TacticalDamageScale;
}

TEnumAsByte<EPlayerBulletType> FBulletInfo::GetBulletType() const
{
    return BulletType;
}

float FBulletInfo::GetRicochetChance() const
{
    return RicochetChance;
}

int32 FBulletInfo::GetNumOfRicochets() const
{
    return NumOfRicochets;
}

float FBulletInfo::GetRicochetMaxDistance() const
{
    return RicochetMaxDistance;
}

float FBulletInfo::GetRicochetDamageScaleMultiply() const
{
    return RicochetDamageScaleMultiply;
}

float FBulletInfo::GetGrenadeRadius() const
{
    return GrenadeRadius;
}

float FBulletInfo::GetGrenadePlayerSelfDamage() const
{
    return GrenadePlayerSelfDamage;
}

FGameplayTag FBulletInfo::GetGrenadeTag() const
{
    return GrenadeTag;
}
#pragma endregion

#pragma region Bullet Pattern
const TArray<FBulletInfo>& FBulletsPattern::GetBullets() const
{
    return Bullets;
}

float FBulletsPattern::GetAttackTime() const
{
    return AttackTime;
}

UAnimMontage* FBulletsPattern::GetAttackMontage() const
{
    return AttackMontage;
}

float FBulletsPattern::GetAttackMontageSpeed() const
{
    return UPlayerWeapon::CalculateMontageSpeed(AttackMontage, bAttackMontageTimeOverride, AttackMontageTimeOverride);
}
#pragma endregion

#pragma region Bullet Pattern Series
const TArray<FBulletsPattern>& FWeaponSeries::GetBulletPatterns() const
{
    return BulletsPatterns;
}

const TArray<TSubclassOf<APickUp>>& FWeaponSeries::GetPickUpsClass() const
{
    return PickUpsClass;
}

float FWeaponSeries::GetPreperationTime() const
{
    return PreperationTime;
}

UAnimMontage* FWeaponSeries::GetPreperationMontage() const
{
    return PreperationMontage;
}

float FWeaponSeries::GetPreperationMontageSpeed() const
{
    return UPlayerWeapon::CalculateMontageSpeed(PreperationMontage, bPreperationMontageTimeOverride, PreperationMontageTimeOverride);
}

float FWeaponSeries::GetRecoveryTime() const
{
    return RecoveryTime;
}

UAnimMontage* FWeaponSeries::GetRecoveryMontage() const
{
    return RecoveryMontage;
}

float FWeaponSeries::GetRecoveryMontageSpeed() const
{
    return UPlayerWeapon::CalculateMontageSpeed(RecoveryMontage, bRecoveryMontageTimeOverride, RecoveryMontageTimeOverride);
}
#pragma endregion

void UPlayerWeapon::PostInitProperties()
{
    Super::PostInitProperties();

    CurrentClipAmmo = MaxClipAmmo;
}

FText UPlayerWeapon::GetWeaponName()
{
    return WeaponName;
}

FGameplayTag UPlayerWeapon::GetWeaponTag()
{
    return WeaponTag;
}

FLinearColor UPlayerWeapon::GetWeaponColor()
{
    return Color;
}

bool UPlayerWeapon::GetIsHidden()
{
    return bIsHidden;
}

void UPlayerWeapon::SetIsHidden(bool IsHidden)
{
    if (bIsHidden != IsHidden)
    {
        if (IsHidden)
        {
            GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

            CurrentClipAmmo = 0;
            CurrentPocketAmmo = 0;
        }
        else
        {
            CurrentClipAmmo = MaxClipAmmo;
        }
    }

    bIsHidden = IsHidden;
}

bool UPlayerWeapon::GetIsSelected()
{
    return bIsSelected;
}

void UPlayerWeapon::SetIsSelected(bool IsSelected)
{
    bIsSelected = IsSelected;
}

float UPlayerWeapon::CalculateMontageSpeed(UAnimMontage* Montage, bool OverrideTime, float TimeOverrideVale)
{
    float MontageSpeed;

    if (!OverrideTime)
    {
        MontageSpeed = 1;
    }
    else
    {
        MontageSpeed = Montage->GetPlayLength() / TimeOverrideVale;
    }

    return MontageSpeed;
}

#pragma region Ammo
bool UPlayerWeapon::CanBeReloaded()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    if (!bIsSelected)
    {
        return false;
    }

    if (bIsThrowingClip)
    {
        return false;
    }

    if (CurrentClipAmmo == MaxClipAmmo)
    {
        return false;
    }

    if (CurrentPocketAmmo <= 0)
    {
        return false;
    }

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (TimerManager.IsTimerActive(AttackTimer))
    {
        return false;
    }

    if (TimerManager.IsTimerActive(ReloadTimer))
    {
        return false;
    }

    return true;
}

void UPlayerWeapon::Reload()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    APlayerCharacter* PlayerCharacter = GetTypedOuter<APlayerCharacter>();
    check(PlayerCharacter);

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    TimerManager.ClearTimer(PreperationTimer);
    TimerManager.ClearTimer(RecoveryTimer);

    if (bCanThrowClip)
    {
        if (!bIsThrowingClip)
        {
            bIsThrowingClip = true;

            CurrentShot = &ClipSeries;

            PreperationStart(true);
            return;
        }
        else
        {
            bIsThrowingClip = false;
        }
    }

    float ReloadTime = 0;
    UAnimMontage* ReloadMontage = 0;
    bool bReloadMontageSpeedOverride = false;
    float ReloadMontageSpeedOverride = 0;

    if (CurrentClipAmmo <= 0)
    {
        ReloadTime = EmptyReloadTime;
        ReloadMontage = EmptyReloadMontage;
        bReloadMontageSpeedOverride = bEmptyReloadMontageTimeOverride;
        ReloadMontageSpeedOverride = EmptyReloadMontageTimeOverride;
    }
    else
    {
        ReloadTime = TacticalReloadTime;
        ReloadMontage = TacticalReloadMontage;
        bReloadMontageSpeedOverride = bTacticalReloadMontageTimeOverride;
        ReloadMontageSpeedOverride = TacticalReloadMontageTimeOverride;
    }

    /*PlayerCharacter->PlayArmsMontage(ReloadMontage, CalculateMontageSpeed(ReloadMontage, bReloadMontageSpeedOverride, ReloadMontageSpeedOverride));*/

    TimerManager.SetTimer(ReloadTimer, this, &UPlayerWeapon::ReloadEnd, ReloadTime);

    PlayerCharacter->UpdateAmmoUI();
}

void UPlayerWeapon::AddAmmo(int32 AmmoAmount)
{
    check(AmmoAmount >= 0);

    CurrentPocketAmmo = FMath::Clamp(CurrentPocketAmmo + AmmoAmount, 0, MaxPocketAmmo);

    if (bCanRegenerateAmmo && CurrentPocketAmmo > AmmoRegenerationTreshold)
    {
        GetWorld()->GetTimerManager().ClearTimer(AmmoRegenerationTimer);
    }

    if (CurrentClipAmmo <= 0 && bIsSelected && CanBeReloaded())
    {
        Reload();
    }

    APlayerCharacter* PlayerCharacter = GetTypedOuter<APlayerCharacter>();
    check(PlayerCharacter);
    PlayerCharacter->UpdateAmmoUI();
}

int32 UPlayerWeapon::GetCurrentClipAmmo()
{
    return CurrentClipAmmo;
}

int32 UPlayerWeapon::GetMaxClipAmmo()
{
    return MaxClipAmmo;
}

int32 UPlayerWeapon::GetCurrentPocketAmmo()
{
    return CurrentPocketAmmo;
}

int32 UPlayerWeapon::GetMaxPocketAmmo()
{
    return MaxPocketAmmo;
}

bool UPlayerWeapon::GetIsReloading()
{
    return GetWorld()->GetTimerManager().IsTimerActive(ReloadTimer);
}

void UPlayerWeapon::ReloadEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    // The Timer must be cleared otherwise the rest of the code doesn't work properly.
    // (UpdateAmmoUI)
    TimerManager.ClearTimer(ReloadTimer);

    if (bResetAttackPatternOnReload)
    {
        CurrentShotIndex = 0;
    }

    int32 MissingClipAmmo = MaxClipAmmo - CurrentClipAmmo;

    if (MissingClipAmmo > CurrentPocketAmmo)
    {
        MissingClipAmmo = CurrentPocketAmmo;
    }

    if (CurrentClipAmmo > 0)
    {
        TacticalAmmo = MissingClipAmmo;
    }
    else
    {
        TacticalAmmo = 0;
    }

    CurrentClipAmmo += MissingClipAmmo;
    CurrentPocketAmmo -= MissingClipAmmo;

    if (bCanRegenerateAmmo && CurrentPocketAmmo <= AmmoRegenerationTreshold)
    {
        FTimerDelegate TimerFunc;
        TimerFunc.BindUObject<UPlayerWeapon, int32>(this, &UPlayerWeapon::AddAmmo, AmmoRegenerationAmount);

        TimerManager.SetTimer(AmmoRegenerationTimer, TimerFunc, AmmoRegenerationTime, true);
    }

    APlayerCharacter* PlayerCharacter = GetTypedOuter<APlayerCharacter>();
    check(PlayerCharacter);
    PlayerCharacter->UpdateAmmoUI();
}
#pragma endregion

#pragma region Attack
bool UPlayerWeapon::CanFire()
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    if (!bIsSelected)
    {
        return false;
    }

    if (CurrentClipAmmo <= 0)
    {
        return false;
    }

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (TimerManager.IsTimerActive(PreperationTimer))
    {
        return false;
    }

    if (TimerManager.IsTimerActive(AttackTimer))
    {
        return false;
    }

    if (TimerManager.IsTimerActive(RecoveryTimer))
    {
        return false;
    }

    if (TimerManager.IsTimerActive(ReloadTimer))
    {
        return false;
    }

    return true;
}

void UPlayerWeapon::Fire()
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    check(ShootingPattern.Num() != 0);

    CurrentShot = &ShootingPattern[CurrentShotIndex];

    CurrentShotIndex++;
    if (CurrentShotIndex >= ShootingPattern.Num())
    {
        CurrentShotIndex = 0;
    }

    PreperationStart(false);
}

void UPlayerWeapon::PreperationStart(bool FromClipThrow)
{
    if (CurrentShot->GetPreperationTime() > 0)
    {
        FTimerDelegate TimerFunc;
        TimerFunc.BindUObject<UPlayerWeapon, bool>(this, &UPlayerWeapon::PreperationEnd, FromClipThrow);

        GetWorld()->GetTimerManager().SetTimer(PreperationTimer, TimerFunc, CurrentShot->GetPreperationTime(), false);

        UAnimMontage* PreperationMontage = CurrentShot->GetPreperationMontage();

        APlayerCharacter* PlayerCharacter = GetTypedOuter<APlayerCharacter>();
        check(PlayerCharacter);

        /*PlayerCharacter->PlayArmsMontage(PreperationMontage, CurrentSeries->GetPreperationMontageSpeed());*/
    }
    else
    {
        PreperationEnd(FromClipThrow);
    }
}

void UPlayerWeapon::PreperationEnd(bool FromClipThrow)
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    CurrentBulletsPatternIndex = 0;

    bool FromTacticalAmmo = TacticalAmmo > 0;

    if (!FromClipThrow)
    {
        CurrentClipAmmo--;
        TacticalAmmo--;
    }

    APlayerCharacter* PlayerCharacter = GetTypedOuter<APlayerCharacter>();
    check(PlayerCharacter);
    PlayerCharacter->UpdateAmmoUI();

    Attack(FromTacticalAmmo, FromClipThrow);
}

void UPlayerWeapon::Attack(bool FromTacticalAmmo, bool FromClipThrow)
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    APlayerCharacter* PlayerCharacter = GetTypedOuter<APlayerCharacter>();
    check(PlayerCharacter);

    if (CurrentBulletsPatternIndex >= CurrentShot->GetBulletPatterns().Num())
    {
        // The Timer must be cleared otherwise the rest of the code doesn't work properly.
        // (CanBeReloaded)
        TimerManager.ClearTimer(AttackTimer);

        if (bIsThrowingClip)
        {
            if (CurrentShot->GetRecoveryTime() > 0)
            {
                TimerManager.SetTimer(RecoveryTimer, this, &UPlayerWeapon::Reload, CurrentShot->GetRecoveryTime());

                UAnimMontage* RecoveryMontage = CurrentShot->GetRecoveryMontage();

                /*PlayerCharacter->PlayArmsMontage(RecoveryMontage, CurrentSeries->GetRecoveryMontageSpeed());*/
            }
            else
            {
                Reload();
            }
        }
        else
        {
            if (CurrentClipAmmo <= 0 && CanBeReloaded())
            {
                Reload();
            }
            else if (CurrentShot->GetRecoveryTime() > 0)
            {
                TimerManager.SetTimer(RecoveryTimer, CurrentShot->GetRecoveryTime(), false);

                UAnimMontage* RecoveryMontage = CurrentShot->GetRecoveryMontage();

                /*PlayerCharacter->PlayArmsMontage(RecoveryMontage, CurrentSeries->GetRecoveryMontageSpeed());*/
            }
        }

        return;
    }
    else
    {
        CurrentBulletsPattern = &CurrentShot->GetBulletPatterns()[CurrentBulletsPatternIndex];
        CurrentBulletsPatternIndex++;
    }

    UAnimMontage* AttackMontage = CurrentBulletsPattern->GetAttackMontage();
    check(AttackMontage);

    PlayerCharacter->PlayArmsMontage(AttackMontage, CurrentBulletsPattern->GetAttackMontageSpeed());

    UCameraComponent* PlayerCamera = PlayerCharacter->GetCameraComponent();
    check(PlayerCamera);

    FRotator CameraRotation = PlayerCamera->GetComponentRotation();
    FVector HitStart = PlayerCamera->GetComponentLocation();
    FVector CameraRightVector = PlayerCamera->GetRightVector();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(PlayerCharacter);
    /*QueryParams.TraceTag = TEXT("TraceTag");
    GetWorld()->DebugDrawTraceTag = TEXT("TraceTag");*/

    for (const FBulletInfo& BulletInfo : CurrentBulletsPattern->GetBullets())
    {

        float DamageScale = 0.0f;

        if (FromTacticalAmmo)
        {
            DamageScale = BulletInfo.GetTacticalDamageScale();
        }
        else
        {
            DamageScale = BulletInfo.GetDamageScale();
        }

        if (FromClipThrow)
        {
            DamageScale *= (1.0f - ((float)CurrentClipAmmo / (float)MaxClipAmmo));
        }

        const AProjectile* ProjectileCDO = GetDefault<AProjectile>(BulletInfo.GetProjectileClass());
        check(ProjectileCDO);

        FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ProjectileCDO->GetSphereRadius());

        FVector HitEnd = BulletInfo.GetTraceDistance() * CameraRotation.Vector() + HitStart + BulletInfo.GetOffsetFromCenter() * CameraRightVector;

        FHitResult Hit;

        if (GetWorld()->SweepSingleByProfile(Hit, HitStart, HitEnd, FQuat(), ProjectileCDO->GetCollisionProfile(), CollisionShape, QueryParams))
        {
            DealDamage(ProjectileCDO, Hit, BulletInfo, CurrentShot->GetPickUpsClass(), DamageScale);
        }
        else
        {
            FActorSpawnParameters ProjectileParams;
            ProjectileParams.Instigator = PlayerCharacter;
            ProjectileParams.Owner = PlayerCharacter;

            FRotator ProjectileRotation = CameraRotation;
            ProjectileRotation.Yaw += FMath::FRandRange(BulletInfo.GetMinRotationOffset().Yaw, BulletInfo.GetMaxRotationOffset().Yaw);
            ProjectileRotation.Pitch += FMath::FRandRange(BulletInfo.GetMinRotationOffset().Pitch, BulletInfo.GetMaxRotationOffset().Pitch);

            AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(BulletInfo.GetProjectileClass(), HitEnd, ProjectileRotation, ProjectileParams);

            FOnProjectileStopAndDestroy OnProjectileStopAndDestoryFunc = FOnProjectileStopAndDestroy::CreateUObject<UPlayerWeapon, const FBulletInfo&, const TArray<TSubclassOf<APickUp>>&, float>(this, &UPlayerWeapon::DealDamage, BulletInfo, CurrentShot->GetPickUpsClass(), DamageScale);

            Projectile->OnProjectileStopAndDestroy = OnProjectileStopAndDestoryFunc;
        }
    }

    FTimerDelegate TimerFunc;
    TimerFunc.BindUObject<UPlayerWeapon, bool, bool>(this, &UPlayerWeapon::Attack, FromTacticalAmmo, FromClipThrow);

    TimerManager.SetTimer(AttackTimer, TimerFunc, CurrentBulletsPattern->GetAttackTime(), false);
}

void UPlayerWeapon::DealDamage(const AProjectile* Projectile, const FHitResult& Hit, const FBulletInfo& BulletInfo, const TArray<TSubclassOf<APickUp>>& PickUpsClass, float DamageScale)
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    APlayerCharacter* PlayerCharacter = GetTypedOuter<APlayerCharacter>();
    check(PlayerCharacter);
    check(Projectile);
    check(Projectile->GetHitParticle());

    UGameplayStatics::SpawnEmitterAttached(Projectile->GetHitParticle(), Hit.GetComponent(), NAME_None, Hit.Location, Hit.Normal.Rotation(), EAttachLocation::KeepWorldPosition);

    if (BulletInfo.GetBulletType() == Grenade)
    {
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Hit.GetActor());

        if (Enemy != nullptr && Enemy->IsAlive())
        {
            Enemy->AddDamage(BulletInfo.GetGrenadeTag(), WeaponTag, DamageScale, PickUpsClass);
        }

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Hit.GetActor());
        QueryParams.AddIgnoredActor(Projectile);
        /*QueryParams.TraceTag = TEXT("TraceTag");
        GetWorld()->DebugDrawTraceTag = TEXT("TraceTag");*/

        DrawDebugSphere(GetWorld(), Hit.Location, BulletInfo.GetGrenadeRadius(), 30, FColor::Red, false, 5.0f);

        FCollisionShape CollisionShape = FCollisionShape::MakeSphere(BulletInfo.GetGrenadeRadius());

        TArray<FOverlapResult> Overlaps;

        if (GetWorld()->OverlapMultiByProfile(Overlaps, Hit.Location, FQuat(), Projectile->GetCollisionProfile(), CollisionShape, QueryParams))
        {
            for (const FOverlapResult& Overlap : Overlaps)
            {
                if (PlayerCharacter == Overlap.GetActor())
                {
                    FVector HitDirection = UKismetMathLibrary::FindLookAtRotation(PlayerCharacter->GetActorLocation(), Hit.Location).Vector();

                    PlayerCharacter->AddPhysicalDamage(BulletInfo.GetGrenadePlayerSelfDamage(), HitDirection);
                    continue;
                }

                /*AEnemyCharacter* */Enemy = Cast<AEnemyCharacter>(Overlap.GetActor());

                if (Enemy != nullptr && Enemy->IsAlive())
                {
                    Enemy->AddDamage(BulletInfo.GetGrenadeTag(), WeaponTag, DamageScale, PickUpsClass);
                }
            }
        }
    }
    else
    {
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Hit.GetActor());

        if (Enemy == nullptr || !Enemy->IsAlive())
        {
            return;
        }

        Enemy->AddDamage(FGameplayTag(), WeaponTag, DamageScale, PickUpsClass);

        if (BulletInfo.GetBulletType() != Ricochet || FMath::FRandRange(0.0001, 0.9999) > BulletInfo.GetRicochetChance())
        {
            return;
        }

        /*DrawDebugSphere(GetWorld(), Hit.Location, BulletInfo.GetRicochetMaxDistance(), 30, FColor::Red, false, 5.0f);*/

        TArray<TTuple<AEnemyCharacter*, FHitResult>, TInlineAllocator<10>> Targets;
        Targets.Reserve(BulletInfo.GetNumOfRicochets());

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(PlayerCharacter);
        QueryParams.AddIgnoredActor(Enemy);
        QueryParams.AddIgnoredActor(Projectile);
        /*QueryParams.TraceTag = TEXT("TraceTag");
        GetWorld()->DebugDrawTraceTag = TEXT("TraceTag");*/

        FCollisionShape CollisionShape = FCollisionShape::MakeSphere(BulletInfo.GetRicochetMaxDistance());

        TArray<FOverlapResult> Overlaps;

        if (!GetWorld()->OverlapMultiByProfile(Overlaps, Hit.Location, FQuat(), Projectile->GetCollisionProfile(), CollisionShape, QueryParams))
        {
            return;
        }

        for (const FOverlapResult& Overlap : Overlaps)
        {
            AEnemyCharacter* Target = Cast<AEnemyCharacter>(Overlap.GetActor());

            if (Target == nullptr || !Target->IsAlive())
            {
                continue;
            }

            FHitResult HitResult;

            if (GetWorld()->LineTraceSingleByProfile(HitResult, Enemy->GetActorLocation(), Target->GetActorLocation(), Projectile->GetCollisionProfile(), QueryParams))
            {
                if (HitResult.GetActor() != Target)
                {
                    continue;
                }
            }

            if (Targets.Num() < BulletInfo.GetNumOfRicochets())
            {
                Targets.Add(TTuple<AEnemyCharacter*, FHitResult>(Target, HitResult));
            }

            for (int32 i = 0; i < Targets.Num(); i++)
            {
                if (Targets[i].Get<0>()->GetCurrentHealth() > Target->GetCurrentHealth())
                {
                    Targets[i] = TTuple<AEnemyCharacter*, FHitResult>(Target, HitResult);
                    break;
                }
            }
        }

        for (TTuple<AEnemyCharacter*, FHitResult> TargetInfo : Targets)
        {
            UGameplayStatics::SpawnEmitterAttached(Projectile->GetHitParticle(), TargetInfo.Get<1>().GetComponent(), NAME_None, TargetInfo.Get<1>().Location, TargetInfo.Get<1>().Normal.Rotation(), EAttachLocation::KeepWorldPosition);

            float Damage = DamageScale;
            Damage *= BulletInfo.GetRicochetDamageScaleMultiply();

            TargetInfo.Get<0>()->AddDamage(FGameplayTag(), WeaponTag, Damage, PickUpsClass);
        }
    }
}

#pragma endregion

#pragma region Shield
bool UPlayerWeapon::IsShieldActive()
{
    return bHasShield && CurrentClipAmmo > 0;
}

float UPlayerWeapon::AbsorbDamage(float DamageToAbsorb)
{
    float AbsorbedDamage = DamageToAbsorb * DamageAbsorption;
    float NewCurrentAbsorbedDamage = CurrentAbsorbedDamage + AbsorbedDamage;

    while (NewCurrentAbsorbedDamage >= MaxAbsorbedDamage)
    {
        NewCurrentAbsorbedDamage -= MaxAbsorbedDamage;

        CurrentClipAmmo -= RemoveAmmoAmount;
    }

    CurrentAbsorbedDamage = NewCurrentAbsorbedDamage;

    if (CurrentClipAmmo <= 0)
    {
        CurrentClipAmmo = 0;

        if (CanBeReloaded())
        {
            Reload();
        }
    }

    APlayerCharacter* PlayerCharacter = GetTypedOuter<APlayerCharacter>();
    check(PlayerCharacter);
    PlayerCharacter->UpdateAmmoUI();

    return DamageToAbsorb - AbsorbedDamage;
}
#pragma endregion

#pragma region Weapon Change
bool UPlayerWeapon::CanBeChangedIn()
{
    if (bIsHidden)
    {
        return false;
    }

    if (GetWorld()->GetTimerManager().IsTimerActive(ReloadTimer))
    {
        return false;
    }

    return true;
}

TTuple<float, UAnimMontage*, float> UPlayerWeapon::ChangeIn()
{
    bIsSelected = true;

    if (bResetAttackPatternOnChangeIn)
    {
        CurrentShotIndex = 0;
    }

    UAnimMontage* Montage = ChangeInMontage;
    float MontageSpeed = CalculateMontageSpeed(Montage, bChangeInMontageTimeOverride, ChangeInMontageTimeOverride);

    return TTuple<float, UAnimMontage*, float>(ChangeInTime, Montage, MontageSpeed);
}

bool UPlayerWeapon::CanBeChangedOut()
{
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (bIsThrowingClip)
    {
        return false;
    }

    /*if (TimerManager.IsTimerActive(PreperationTimer))
    {
        return false;
    }*/

    if (TimerManager.IsTimerActive(AttackTimer))
    {
        return false;
    }

    /*if (TimerManager.IsTimerActive(RecoveryTimer))
    {
        return false;
    }*/

    /*if (TimerManager.IsTimerActive(ReloadTimer))
    {
        return false;
    }*/

    return true;
}

TTuple<float, UAnimMontage*, float> UPlayerWeapon::ChangeOut()
{
    bIsSelected = false;

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    TimerManager.ClearTimer(PreperationTimer);

    if (TimerManager.IsTimerActive(ReloadTimer))
    {
        float ReloadTime = TimerManager.GetTimerRemaining(ReloadTimer);
        ReloadTime += PocketReloadAdditionalTime;

        TimerManager.SetTimer(ReloadTimer, this, &UPlayerWeapon::ReloadEnd, ReloadTime);
    }
    else
    {
        TimerManager.ClearTimer(RecoveryTimer);
        TimerManager.ClearTimer(PreperationTimer);
    }

    UAnimMontage* Montage = ChangeOutMontage;
    float MontageSpeed = CalculateMontageSpeed(Montage, bChangeOutMontageTimeOverride, ChangeOutMontageTimeOverride);

    return TTuple<float, UAnimMontage*, float>(ChangeOutTime, Montage, MontageSpeed);
}

#pragma endregion
