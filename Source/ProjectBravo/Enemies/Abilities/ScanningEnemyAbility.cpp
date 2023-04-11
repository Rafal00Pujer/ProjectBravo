// Fill out your copyright notice in the Description page of Project Settings.


#include "ScanningEnemyAbility.h"
#include "Kismet/KismetMathLibrary.h"
#include <ProjectBravo/Player/PlayerCharacter.h>

void UScanningEnemyAbility::PostInitProperties()
{
    Super::PostInitProperties();

    if (EnemyController == nullptr)
    {
        return;
    }

    UObject* ScanningStartPointObject = EnemyCharacter->GetDefaultSubobjectByName(ScanningStartPointSceneComponentName);
    check(ScanningStartPointObject);

    ScanningStartPoint = Cast<USceneComponent>(ScanningStartPointObject);
    checkf(ScanningStartPoint, TEXT("Scanning Start Point not found."));
}

void UScanningEnemyAbility::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    DrawAbilitDebugLine(DeltaTime);
}

void UScanningEnemyAbility::PreperationEnd()
{
    Super::PreperationEnd();

    ScanningRotationToPlayer = UKismetMathLibrary::FindLookAtRotation(ScanningStartPoint->GetComponentLocation(), PlayerCharacter->GetActorLocation());
}

void UScanningEnemyAbility::AttackDelayEnd()
{
    Super::AttackDelayEnd();

    if (bSetFocusInDelayAttackAndAttack)
    {
        ScanningRotationToPlayer = UKismetMathLibrary::FindLookAtRotation(ScanningStartPoint->GetComponentLocation(), PlayerCharacter->GetActorLocation());
    }

    FVector HitStart = ScanningStartPoint->GetComponentLocation();
    FVector HitEnd = HitStart + ScanningRotationToPlayer.Vector() * MaxScanningDistance;

    FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ScanningSphereRadius);

    FCollisionQueryParams QueryParams;
    /*QueryParams.TraceTag = TEXT("TraceTag");
    GetWorld()->DebugDrawTraceTag = TEXT("TraceTag");*/

    FHitResult Hit;

    if (GetWorld()->SweepSingleByChannel(Hit, HitStart, HitEnd, FQuat(), ScanningTraceChannel, CollisionShape, QueryParams))
    {
        PlayerCharacter->AddPhysicalDamage(Damage, Hit.ImpactNormal);

        TryToSpawnAOE(Hit.Location);
    }
}

void UScanningEnemyAbility::DrawAbilitDebugLine(float DeltaTime)
{
#if WITH_EDITOR

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    FColor LineColor;
    FRotator RotationToPlayer;

    if (TimerManager.IsTimerActive(PreparationTimer))
    {
        LineColor = FColor::Yellow;

        RotationToPlayer = UKismetMathLibrary::FindLookAtRotation(ScanningStartPoint->GetComponentLocation(), PlayerCharacter->GetActorLocation());

        goto SkipSetRotationToPlayer;
    }
    else  if (TimerManager.IsTimerActive(AttackDelayTimer))
    {
        LineColor = FColor::Orange;
    }
    else
    {
        LineColor = FColor::Red;
    }

    if (bSetFocusInDelayAttackAndAttack)
    {
        RotationToPlayer = UKismetMathLibrary::FindLookAtRotation(ScanningStartPoint->GetComponentLocation(), PlayerCharacter->GetActorLocation());
    }
    else
    {
        RotationToPlayer = ScanningRotationToPlayer;
    }

SkipSetRotationToPlayer:;

    FVector LineStart = ScanningStartPoint->GetComponentLocation();
    FVector LineEnd = LineStart + RotationToPlayer.Vector() * MaxScanningDistance;

    DrawDebugLine(GetWorld(), LineStart, LineEnd, LineColor, false, DeltaTime * 2.0f, 1U, 10.0f);

#endif // WITH_EDITOR
}
