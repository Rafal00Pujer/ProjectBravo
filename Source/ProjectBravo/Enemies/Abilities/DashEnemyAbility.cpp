// Fill out your copyright notice in the Description page of Project Settings.


#include "DashEnemyAbility.h"
#include <ProjectBravo/Player/PlayerCharacter.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h" 

void UDashEnemyAbility::PostInitProperties()
{
    Super::PostInitProperties();

    DashSpeed = DashDistance / DashTime;
    StunTime = AttackTime - DashTime;
}

void UDashEnemyAbility::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UDashEnemyAbility, DashTime) || PropertyName == GET_MEMBER_NAME_CHECKED(UDashEnemyAbility, AttackTime))
    {
        DashTime = FMath::Clamp(DashTime, 0.0001f, AttackTime);
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UDashEnemyAbility, AbilityMovementType))
    {
        AbilityMovementType = EStandardMovementType::StopMovement;
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool UDashEnemyAbility::CanBeActivated()
{
    if (!Super::CanBeActivated())
    {
        return false;
    }

    FAIMoveRequest MoveReq(PlayerCharacter);
    FPathFindingQuery PFQuery;

    if (!EnemyController->BuildPathfindingQuery(MoveReq, PFQuery))
    {
        return false;
    }

    FNavPathSharedPtr Path;
    EnemyController->FindPathForMoveRequest(MoveReq, PFQuery, Path);

    float DistanceToPlayer = FVector::Dist(EnemyCharacter->GetActorLocation(), PlayerCharacter->GetActorLocation());

    if (Path.IsValid() && Path.Get()->GetLength() > DistanceToPlayer + MaxDistanceDifferenceForPathCheck)
    {
        return false;
    }

    return true;
}

void UDashEnemyAbility::Activate()
{
    Super::Activate();

    BrakingFrictionFactorSave = EnemyCharacter->GetCharacterMovement()->BrakingFrictionFactor;
    BrakingDecelerationWalkingSave = EnemyCharacter->GetCharacterMovement()->BrakingDecelerationWalking;
}

bool UDashEnemyAbility::CanTick()
{
    if (!Super::CanTick())
    {
        return false;
    }

    if (!GetWorld()->GetTimerManager().IsTimerActive(DashTimer))
    {
        return false;
    }

    return true;
}

void UDashEnemyAbility::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector EnemyLocation = EnemyCharacter->GetActorLocation();

    TryToSpawnAOE(EnemyLocation);

    FVector& TraceStart = EnemyLocation;
    FVector TraceEnd = EnemyCharacter->GetActorForwardVector() * TraceDistance + TraceStart;

    UCapsuleComponent* EnemyCapsule = EnemyCharacter->GetCapsuleComponent();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(EnemyCharacter);
    /*QueryParams.TraceTag = TEXT("TraceTag");
    GetWorld()->DebugDrawTraceTag = TEXT("TraceTag");*/

    FHitResult Hit;

    if (GetWorld()->SweepSingleByProfile(Hit, TraceStart, TraceEnd, EnemyCapsule->GetComponentQuat(), DodgeCollisionPreset, EnemyCapsule->GetCollisionShape(EnemyCapsuleInflation), QueryParams))
    {
        //UE_LOG(LogTemp, Warning, TEXT("Dash hit. Actor = %s."), *Hit.GetActor()->GetName());

        APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Hit.GetActor());

        if (PlayerChar != nullptr)
        {
            PlayerChar->AddPhysicalDamage(Damage, Hit.ImpactNormal);

            GetWorld()->GetTimerManager().ClearTimer(AttackTimer);
            AttackEnd();
        }
        else if (StunTime > 0)
        {
            GetWorld()->GetTimerManager().SetTimer(AttackTimer, this, &UDashEnemyAbility::AttackEnd, StunTime);
        }

        GetWorld()->GetTimerManager().ClearTimer(DashTimer);
        DashEnd();
    }
}

void UDashEnemyAbility::Abort()
{
    Super::Abort();

    GetWorld()->GetTimerManager().ClearTimer(DashTimer);
    DashEnd();
}

void UDashEnemyAbility::AttackDelayEnd()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    Super::AttackDelayEnd();

    EnemyCharacter->GetCharacterMovement()->BrakingFrictionFactor = 0;
    EnemyCharacter->GetCharacterMovement()->BrakingDecelerationWalking = 0;

    EnemyController->ClearFocus(EAIFocusPriority::Gameplay);

    EnemyCharacter->LaunchCharacter(EnemyCharacter->GetActorForwardVector() * DashSpeed, true, false);

    GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &UDashEnemyAbility::DashEnd, DashTime);
}

void UDashEnemyAbility::DashEnd()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    EnemyCharacter->LaunchCharacter(FVector::ZeroVector, true, false);

    EnemyCharacter->GetCharacterMovement()->BrakingFrictionFactor = BrakingFrictionFactorSave;
    EnemyCharacter->GetCharacterMovement()->BrakingDecelerationWalking = BrakingDecelerationWalkingSave;
}
