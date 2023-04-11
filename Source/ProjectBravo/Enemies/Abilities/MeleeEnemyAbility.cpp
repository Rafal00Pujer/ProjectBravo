// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemyAbility.h"
#include <ProjectBravo/Player/PlayerCharacter.h>
#include "Components/SplineComponent.h" 

void UMeleeEnemyAbility::PostInitProperties()
{
    Super::PostInitProperties();

    if (EnemyController == nullptr)
    {
        return;
    }

    UObject* MelleSplineComponentObject = EnemyCharacter->GetDefaultSubobjectByName(MeleeSplineComponentName);
    check(MelleSplineComponentObject);

    MeleeSplineComponent = Cast<USplineComponent>(MelleSplineComponentObject);
    checkf(MeleeSplineComponent, TEXT("Melle Spline Component not found."));

    if (MeleeSplineComponent->Duration != AttackTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Melee spline duration != Attack time. Duration set to Attack time."));

        MeleeSplineComponent->Duration = AttackTime;
    }

    MeleeCollision = FCollisionShape::MakeSphere(MelleSphereRadius);
}

void UMeleeEnemyAbility::Activate()
{
    Super::Activate();

    bPlayerIsHit = false;
    CurrentSplineTime = 0.0f;
}

bool UMeleeEnemyAbility::CanTick()
{
    if (!Super::CanTick())
    {
        return false;
    }

    if (!bStartSplineInterp || bPlayerIsHit)
    {
        return false;
    }

    return true;
}

void UMeleeEnemyAbility::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector CurrentSplinePosition = MeleeSplineComponent->GetLocationAtTime(CurrentSplineTime, ESplineCoordinateSpace::World);

    CurrentSplineTime += DeltaTime;

    if (CurrentSplineTime > AttackTime)
    {
        CurrentSplineTime = AttackTime;
    }

    FVector NextSplinePosition = MeleeSplineComponent->GetLocationAtTime(CurrentSplineTime, ESplineCoordinateSpace::World);

    TryToSpawnAOE(NextSplinePosition);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(EnemyCharacter);
    QueryParams.TraceTag = TEXT("TraceTag");
    GetWorld()->DebugDrawTraceTag = TEXT("TraceTag");

    TArray<FHitResult> Hits;

    if (GetWorld()->SweepMultiByProfile(Hits, CurrentSplinePosition, NextSplinePosition, FQuat(), MeleeTraceProfile, MeleeCollision, QueryParams))
    {
        for (const FHitResult& Hit : Hits)
        {
            APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Hit.GetActor());

            if (PlayerChar == nullptr)
            {
                continue;
            }

            PlayerChar->AddPhysicalDamage(Damage, Hit.ImpactNormal);

            if (bCanImmobilize)
            {
                PlayerChar->Immobilize(ImmobilizeTime);
            }

            bPlayerIsHit = true;
            return;
        }
    }
}

void UMeleeEnemyAbility::AttackDelayEnd()
{
    Super::AttackDelayEnd();

    bStartSplineInterp = true;
}

void UMeleeEnemyAbility::AttackEnd()
{
    Super::AttackEnd();

    bStartSplineInterp = false;
}
