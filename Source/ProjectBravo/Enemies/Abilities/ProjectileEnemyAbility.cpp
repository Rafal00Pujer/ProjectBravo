// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileEnemyAbility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include <ProjectBravo/Player/PlayerCharacter.h>

void UProjectileEnemyAbility::PostInitProperties()
{
    Super::PostInitProperties();

    if (EnemyController == nullptr)
    {
        return;
    }

    UObject* ScanningStartPointObject = EnemyCharacter->GetDefaultSubobjectByName(ProjectileStartPointSceneComponentName);
    check(ScanningStartPointObject);

    ProjectileStartPoint = Cast<USceneComponent>(ScanningStartPointObject);
    checkf(ProjectileStartPoint, TEXT("Projectile Start Point not found."));
}

void UProjectileEnemyAbility::PreperationEnd()
{
    Super::PreperationEnd();

    ProjectileTargetPoint = PlayerCharacter->GetActorLocation();
}

void UProjectileEnemyAbility::AttackDelayEnd()
{
    Super::AttackDelayEnd();

    if (bSetFocusInDelayAttackAndAttack)
    {
        ProjectileTargetPoint = PlayerCharacter->GetActorLocation();
    }

    AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, ProjectileStartPoint->GetComponentTransform());

    if (Projectile == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Projectile is null!!! But why????"));
        return;
    }

    Projectile->OnProjectileStopAndDestroy.BindUObject(this, &UProjectileEnemyAbility::DealDamage);

    UProjectileMovementComponent* ProjectileMovement = Projectile->GetProjectileMovementComponent();
    check(ProjectileMovement);

    ProjectileTargetPoint.X += FMath::FRandRange(ProjectileMinTargetOffset.X, ProjectileMaxTargetOffset.X);
    ProjectileTargetPoint.Y += FMath::FRandRange(ProjectileMinTargetOffset.Y, ProjectileMaxTargetOffset.Y);
    ProjectileTargetPoint.Z += FMath::FRandRange(ProjectileMinTargetOffset.Z, ProjectileMaxTargetOffset.Z);

    FVector ProjectileVelocity;
    UGameplayStatics::SuggestProjectileVelocity(this, ProjectileVelocity, ProjectileStartPoint->GetComponentLocation(), ProjectileTargetPoint, ProjectileMovement->InitialSpeed, false, 0.0f, ProjectileMovement->GetGravityZ(), ESuggestProjVelocityTraceOption::DoNotTrace);

    Projectile->GetProjectileMovementComponent()->Velocity = ProjectileVelocity;

    /*UE_LOG(LogTemp, Warning, TEXT("Projectile Movement Initial Speed = %f"), ProjectileMovement->InitialSpeed);
    UE_LOG(LogTemp, Warning, TEXT("Projectile Velocity = %s"), *ProjectileVelocity.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Projectile Target Point = %s"), *ProjectileTargetPoint.ToString());*/
}

void UProjectileEnemyAbility::DealDamage(const AProjectile* Projectile, const FHitResult& ImpactResult)
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
    UE_LOG(LogTemp, Warning, TEXT("Impact Result Get Actor = %s"), *ImpactResult.GetActor()->GetName());*/

    TryToSpawnAOE(Projectile->GetActorLocation());

    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(ImpactResult.GetActor());

    if (PlayerChar != nullptr)
    {
        PlayerChar->AddPhysicalDamage(Damage, ImpactResult.ImpactNormal);
    }

    if (bIsGrenade)
    {
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(ImpactResult.GetActor());
        QueryParams.AddIgnoredActor(Projectile);
        /*QueryParams.TraceTag = TEXT("TraceTag");
        GetWorld()->DebugDrawTraceTag = TEXT("TraceTag");*/

        DrawDebugSphere(GetWorld(), ImpactResult.Location, GrenadeRadius, 30, FColor::Red, false, 5.0f);

        FCollisionShape CollisionShape = FCollisionShape::MakeSphere(GrenadeRadius);

        TArray<FOverlapResult> Overlaps;

        if (GetWorld()->OverlapMultiByProfile(Overlaps, ImpactResult.Location, FQuat(), Projectile->GetCollisionProfile(), CollisionShape, QueryParams))
        {
            for (const FOverlapResult& Overlap : Overlaps)
            {
                PlayerChar = Cast<APlayerCharacter>(Overlap.GetActor());

                if (PlayerChar != nullptr)
                {
                    FVector HitDirection = UKismetMathLibrary::FindLookAtRotation(PlayerChar->GetActorLocation(), ImpactResult.Location).Vector();

                    PlayerChar->AddPhysicalDamage(Damage, HitDirection);
                    return;
                }
            }
        }
    }
}
