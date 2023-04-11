// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAbility.h"
#include <ProjectBravo/Player/PlayerCharacter.h>
#include "ProjectBravo/Enemies/EnemyController.h"
#include <ProjectBravo/Enemies/EnemyCharacter.h>
#include "EnemyAOE.h"
#include "Components/SphereComponent.h"

void UEnemyAbility::PostInitProperties()
{
    Super::PostInitProperties();

    if (FirstCooldown > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &UEnemyAbility::FirstCooldownEnd, FirstCooldown);
    }

    if (NextAbilityClass != nullptr)
    {
        NextAbility = NewObject<UEnemyAbility>(this, NextAbilityClass);
        check(NextAbility);
    }

    EnemyController = GetTypedOuter<AEnemyController>();

    if (EnemyController == nullptr)
    {
        return;
    }

    APawn* EnemyPawn = EnemyController->GetPawn();
    check(EnemyPawn);
    EnemyCharacter = Cast<AEnemyCharacter>(EnemyPawn);
    check(EnemyCharacter);

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    check(PlayerController);
    APawn* PlayerPawn = PlayerController->GetPawn();
    check(PlayerPawn);
    PlayerCharacter = Cast<APlayerCharacter>(PlayerPawn);
    check(PlayerCharacter);

    if (bCreateAOE)
    {
        check(AOEClass);
    }
}

bool UEnemyAbility::CanBeActivated()
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (TimerManager.IsTimerActive(CooldownTimer))
    {
        return false;
    }

    /*if (TimerManager.IsTimerActive(PreparationTimer))
    {
        return false;
    }

    if (TimerManager.IsTimerActive(AttackDelayTimer))
    {
        return false;
    }

    if (TimerManager.IsTimerActive(AttackTimer))
    {
        return false;
    }*/

    if (bIsCurrentlyActive)
    {
        return false;
    }

    if (!EnemyController->LineOfSightTo(PlayerCharacter))
    {
        return false;
    }

    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    FVector EnemyLocation = EnemyCharacter->GetActorLocation();

    float DistanceToPlayer = FVector::Dist(PlayerLocation, EnemyLocation);

    if (DistanceToPlayer > MaxActivationDistance)
    {
        return false;
    }

    if (DistanceToPlayer < MinActivationDistance)
    {
        return false;
    }

    /*if (OnCanBeActivated.IsBound())
    {
        if (!OnCanBeActivated.Execute(this))
        {
            return false;
        }
    }*/

    return true;
}

void UEnemyAbility::Activate()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    bIsCurrentlyActive = true;

    /*OnActivate.ExecuteIfBound(this);*/

    EnemyController->SetFocus(PlayerCharacter, EAIFocusPriority::Gameplay);

    if (PreperationTime > 0)
    {
        DrawAbilityDebugBox(FColor::Yellow, PreperationTime);

        GetWorld()->GetTimerManager().SetTimer(PreparationTimer, this, &UEnemyAbility::PreperationEnd, PreperationTime);

        EnemyCharacter->PlayAnimMontage(PreperationMontage);
        return;
    }

    PreperationEnd();
}

bool UEnemyAbility::CanTick()
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    return /*bCanTick &&*/ bIsCurrentlyActive;
}

void UEnemyAbility::Tick(float DeltaTime)
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

void UEnemyAbility::Abort()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    /*OnAbort.ExecuteIfBound(this);*/

    bIsCurrentlyActive = false;

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    TimerManager.ClearTimer(PreparationTimer);
    TimerManager.ClearTimer(AttackDelayTimer);
    TimerManager.ClearTimer(AttackTimer);

    if (!TimerManager.IsTimerActive(CooldownTimer) && Cooldown > 0)
    {
        TimerManager.SetTimer(CooldownTimer, this, &UEnemyAbility::CooldownEnd, Cooldown);
    }

    EnemyCharacter->StopAnimMontage(PreperationMontage);
    EnemyCharacter->StopAnimMontage(AttackDelayMontage);
    EnemyCharacter->StopAnimMontage(AttackMontage);
}

int32 UEnemyAbility::GetActivationPriority()
{
    return ActivationPriority;
}

TEnumAsByte<EStandardMovementType> UEnemyAbility::GetAbilityMovementType()
{
    return AbilityMovementType;
}

UEnemyAbility* UEnemyAbility::GetNextAbility()
{
    return NextAbility;
}

float UEnemyAbility::GetEntireAbilityTime()
{
    return PreperationTime + AttackDelay + AttackTime;
}

void UEnemyAbility::FirstCooldownEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

void UEnemyAbility::CooldownEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

void UEnemyAbility::PreperationEnd()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    /*OnPreperationEnd.ExecuteIfBound(this);*/

    if (!bSetFocusInDelayAttackAndAttack)
    {
        AActor* FocusActor = EnemyController->GetFocusActorForPriority(EAIFocusPriority::Gameplay);
        check(FocusActor);

        UE_LOG(LogTemp, Warning, TEXT("Focus Actor name = %s"), *FocusActor->GetName());

        EnemyController->SetFocalPoint(FocusActor->GetActorLocation(), EAIFocusPriority::Gameplay);
    }

    if (AttackDelay > 0)
    {
        DrawAbilityDebugBox(FColor::Orange, AttackDelay);

        GetWorld()->GetTimerManager().SetTimer(AttackDelayTimer, this, &UEnemyAbility::AttackDelayEnd, AttackDelay);

        EnemyCharacter->PlayAnimMontage(AttackDelayMontage);
        return;
    }

    AttackDelayEnd();
}

void UEnemyAbility::AttackDelayEnd()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    /*OnAttackDelayEnd.ExecuteIfBound(this);*/

    DrawAbilityDebugBox(FColor::Red, AttackTime);

    GetWorld()->GetTimerManager().SetTimer(AttackTimer, this, &UEnemyAbility::AttackEnd, AttackTime);

    EnemyCharacter->PlayAnimMontage(AttackMontage);
}

void UEnemyAbility::AttackEnd()
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    EnemyController->ClearFocus(EAIFocusPriority::Gameplay);

    if (Cooldown > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &UEnemyAbility::CooldownEnd, Cooldown);
    }

    bIsCurrentlyActive = false;

    OnAttackEnd.ExecuteIfBound(this);
}

bool UEnemyAbility::TryToSpawnAOE(FVector ZPlaneOrigin, AEnemyAOE*& OutAOE)
{
    if (!bCreateAOE)
    {
        return false;
    }

    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    checkSlow(AOEClass);

    const AEnemyAOE* AOE_CDO = GetDefault<AEnemyAOE>(AOEClass);
    check(AOE_CDO);

    const USphereComponent* AOESphere = AOE_CDO->GetAOECollision();
    check(AOESphere);

    FVector& TraceStart = ZPlaneOrigin;
    FVector TraceEnd = TraceStart;
    TraceEnd.Z -= 10000;

    FCollisionQueryParams QueryParams;
    /*QueryParams.AddIgnoredActor(EnemyCharacter);
    QueryParams.AddIgnoredActor(PlayerCharacter);*/
    /*QueryParams.TraceTag = TEXT("TraceTag");
    GetWorld()->DebugDrawTraceTag = TEXT("TraceTag");*/

    FHitResult Hit;

    if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, AOESphere->GetComponentQuat(), AOECollisionCheckTrace, AOESphere->GetCollisionShape(), QueryParams))
    {
        UE_LOG(LogTemp, Warning, TEXT("AOE is already in place!"));
        return false;
    }

    OutAOE = GetWorld()->SpawnActor<AEnemyAOE>(AOEClass, ZPlaneOrigin, FRotator());
    check(OutAOE);
    return true;
}

bool UEnemyAbility::TryToSpawnAOE(FVector ZPlaneOrigin)
{
    AEnemyAOE* AOE;
    return TryToSpawnAOE(ZPlaneOrigin, AOE);
}

void UEnemyAbility::DrawAbilityDebugBox(FColor BoxColor, float DrawTime)
{
#if WITH_EDITOR

    FVector DebugLocation = EnemyCharacter->GetActorLocation();
    DebugLocation.Z += 120.0f;
    DrawDebugSolidBox(GetWorld(), DebugLocation, FVector(21.0, 21.0, 21.0), BoxColor, false, DrawTime);

#endif // WITH_EDITOR
}
