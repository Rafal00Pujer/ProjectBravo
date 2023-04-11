// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"
#include <NavigationSystem.h>
#include "Abilities/EnemyAbility.h"
#include "Abilities/ProjectileEnemyAbility.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

void AEnemyController::BeginPlay()
{
    Super::BeginPlay();

    EnemyCharacter = GetPawn<AEnemyCharacter>();
    check(EnemyCharacter);

    EnemyCharacter->OnEnemyAddDamage.BindUObject(this, &AEnemyController::OnEnemyAddDamage);

    GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &AEnemyController::OnMovementRequestFinished);

    Abilities.Reset(AbilitiesClass.Num());

    for (TSubclassOf<UEnemyAbility> AbilityClass : AbilitiesClass)
    {
        UEnemyAbility* NewAbility = NewObject<UEnemyAbility>(this, AbilityClass);
        check(NewAbility);
        Abilities.Add(NewAbility);

        while (NewAbility != nullptr)
        {
            NewAbility->OnAttackEnd.BindUObject(this, &AEnemyController::OnAbilityAttackEnd);
            NewAbility = NewAbility->GetNextAbility();
        }
    }

    Abilities.Sort([](UEnemyAbility& A, UEnemyAbility& B)
        {
            return A.GetActivationPriority() > B.GetActivationPriority();
        });

    GetWorldTimerManager().SetTimer(NextAbilityActivationTryTimer, this, &AEnemyController::ActivateNewAbility, SpawnAbilityActivationDelay);

    if (bCanDodge)
    {
        DodgeSpeed = DodgeDistance / DodgeTime;

        if (DodgeFirstCooldown > 0)
        {
            GetWorldTimerManager().SetTimer(DodgeCooldownTimer, DodgeFirstCooldown, false);
        }

        if (DodgeParallelAbilityClass != nullptr)
        {
            UEnemyAbility* DodgeParallelAbilityLoc = NewObject<UEnemyAbility>(this, DodgeParallelAbilityClass);
            check(DodgeParallelAbilityLoc);

            DodgeParallelAbility = DodgeParallelAbilityLoc;

            while (DodgeParallelAbilityLoc != nullptr)
            {
                checkf(DodgeParallelAbilityLoc->GetAbilityMovementType() == EStandardMovementType::StopMovement, TEXT("Dodge parallel ability movement type != Stop Movement."));

                DodgeParallelAbilityLoc->OnAttackEnd.BindUObject(this, &AEnemyController::OnAbilityAttackEnd);
                DodgeParallelAbilityLoc = DodgeParallelAbilityLoc->GetNextAbility();
            }
        }
    }
}

void AEnemyController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentAbility != nullptr && CurrentAbility->CanTick())
    {
        CurrentAbility->Tick(DeltaTime);
    }
}

void AEnemyController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
    APawn* const MyPawn = GetPawn();
    if (MyPawn)
    {
        FRotator NewControlRotation = GetControlRotation();

        // Look toward focus
        const FVector FocalPoint = GetFocalPoint();
        if (FAISystem::IsValidLocation(FocalPoint))
        {
            NewControlRotation = (FocalPoint - MyPawn->GetPawnViewLocation()).Rotation();
        }
        else if (bSetControlRotationFromPawnOrientation)
        {
            NewControlRotation = MyPawn->GetActorRotation();
        }

        // Remove form orginal function.
        // Don't pitch view unless looking at another pawn
        /*if (NewControlRotation.Pitch != 0 && Cast<APawn>(GetFocusActor()) == nullptr)
        {
            NewControlRotation.Pitch = 0.f;
        }*/

        SetControlRotation(NewControlRotation);

        if (bUpdatePawn)
        {
            const FRotator CurrentPawnRotation = MyPawn->GetActorRotation();

            if (CurrentPawnRotation.Equals(NewControlRotation, 1e-3f) == false)
            {
                MyPawn->FaceRotation(NewControlRotation, DeltaTime);
            }
        }
    }
}

void AEnemyController::Destroyed()
{
    SetCanActivateAbilities(false);

    Super::Destroyed();
}

void AEnemyController::OnEnemyAddDamage(FGameplayTag GrenadeTag, FGameplayTag WeaponTag, float DamageScale, float Damage)
{
    DodgeStart();
}

#pragma region Abilities
void AEnemyController::SetCanActivateAbilities(bool NewCanActivateAbilities)
{
    if (NewCanActivateAbilities == bCanActivateAbilities)
    {
        return;
    }

    bCanActivateAbilities = NewCanActivateAbilities;

    if (bCanActivateAbilities)
    {
        ActivateNewAbility();
    }
    else if (CurrentAbility != nullptr)
    {
        CurrentAbility->Abort();
        CurrentAbility = nullptr;
    }
}

void AEnemyController::ActivateNewAbility()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    if (IsDodging())
    {
        return;
    }

    if (bCanActivateAbilities)
    {
        for (UEnemyAbility* Ability : Abilities)
        {
            if (Ability->CanBeActivated())
            {
                CurrentAbility = Ability;

                ActivateCurrentAbility();

                return;
            }
        }
    }

    if (CurrentMovementType != DefaultMovementType)
    {
        CurrentMovementType = DefaultMovementType;
        HandleMovement();
    }

    GetWorldTimerManager().SetTimer(NextAbilityActivationTryTimer, this, &AEnemyController::ActivateNewAbility, DelayForNextAbilityActivationTry);
}

void AEnemyController::ActivateCurrentAbility()
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    CurrentAbility->Activate();

    if (CurrentMovementType != CurrentAbility->GetAbilityMovementType())
    {
        CurrentMovementType = CurrentAbility->GetAbilityMovementType();
        HandleMovement();
    }
}

void AEnemyController::OnAbilityAttackEnd(UEnemyAbility* Ability)
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    if (Ability->GetNextAbility() != nullptr && Ability->GetNextAbility()->CanBeActivated())
    {
        CurrentAbility = Ability->GetNextAbility();

        ActivateCurrentAbility();

        return;
    }

    CurrentAbility = nullptr;

    ActivateNewAbility();
}
#pragma endregion

#pragma region Movement
void AEnemyController::HandleMovement()
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    FTimerManager& TimerManager = GetWorldTimerManager();
    TimerManager.ClearTimer(DelayForNextMovementRequestOnFailTimer);
    TimerManager.ClearTimer(DelayForNextMovementRequestTimer);

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    check(PlayerController);
    APawn* PlayerPawn = PlayerController->GetPawn();
    check(PlayerPawn);

    FNavLocation RandomNavLoc;
    const UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

    switch (CurrentMovementType)
    {
    case EStandardMovementType::StopMovement:
        StopMovement();
        return;

    case RandomAroundPlayer:
        if (NavSystem->GetRandomReachablePointInRadius(PlayerPawn->GetActorLocation(), RandomAroundPlayerRadius, RandomNavLoc))
        {
            if (MoveToLocation(RandomNavLoc.Location) == EPathFollowingRequestResult::RequestSuccessful)
            {
                return;
            }
        }

        break;

    case RandomOnLevel:
        if (NavSystem->GetRandomReachablePointInRadius(GetPawn()->GetActorLocation(), RandomOnLevelRadius, RandomNavLoc))
        {
            if (MoveToLocation(RandomNavLoc.Location) == EPathFollowingRequestResult::RequestSuccessful)
            {
                return;
            }
        }

        break;

    case MoveToPlayer:
        if (MoveToActor(PlayerPawn, MoveToPlayerAcceptanceRadius) == EPathFollowingRequestResult::RequestSuccessful)
        {
            return;
        }

        break;

    default:
        break;
    }

    TimerManager.SetTimer(DelayForNextMovementRequestOnFailTimer, this, &AEnemyController::HandleMovement, DelayForNextMovementRequestOnFail);
}

void AEnemyController::OnMovementRequestFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    if (Result.IsSuccess())
    {
        GetWorldTimerManager().SetTimer(DelayForNextMovementRequestTimer, this, &AEnemyController::HandleMovement, DelayForNextMovementRequest);

        return;
    }

    GetWorldTimerManager().SetTimer(DelayForNextMovementRequestOnFailTimer, this, &AEnemyController::HandleMovement, DelayForNextMovementRequestOnFail);
}
#pragma endregion

#pragma region Dodge
bool AEnemyController::IsDodging()
{
    return GetWorldTimerManager().IsTimerActive(DodgeTimer);
}

void AEnemyController::DodgeStart()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    if (!bCanDodge)
    {
        return;
    }

    if (IsDodging())
    {
        return;
    }

    FTimerManager& TimerManager = GetWorldTimerManager();

    if (TimerManager.IsTimerActive(DodgeCooldownTimer))
    {
        return;
    }

    if (CurrentAbility != nullptr)
    {
        CurrentAbility->Abort();
    }

    if (CurrentMovementType != EStandardMovementType::StopMovement)
    {
        CurrentMovementType = EStandardMovementType::StopMovement;
        HandleMovement();
    }

    ClearFocus(EAIFocusPriority::Gameplay);

    if (DodgeParallelAbility != nullptr && DodgeParallelAbility->CanBeActivated())
    {
        CurrentAbility = DodgeParallelAbility;
        ActivateCurrentAbility();
    }

    TimerManager.SetTimer(DodgeTimer, this, &AEnemyController::DodgeEnd, DodgeTime);

    BrakingFrictionFactorSave = EnemyCharacter->GetCharacterMovement()->BrakingFrictionFactor;
    BrakingDecelerationWalkingSave = EnemyCharacter->GetCharacterMovement()->BrakingDecelerationWalking;

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    check(PlayerController);
    APawn* PlayerPawn = PlayerController->GetPawn();
    check(PlayerPawn);

    FVector RightVector = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::FindLookAtRotation(EnemyCharacter->GetActorLocation(), PlayerPawn->GetActorLocation()));

    float Direction = FMath::RandBool() ? -1.0f : 1.0f;
    float OppositeDirection = Direction * -1.0f;

    // Take a clear direction or dodge without moving.
    /*if (!IsDodgePathClear(Direction, RightVector))
    {
        Direction *= -1.0f;

        if (!IsDodgePathClear(Direction, RightVector))
        {
            EnemyCharacter->PlayAnimMontage(DodgeNoMoveMontage);

            UE_LOG(LogTemp, Warning, TEXT("Obstruction on dodge path!"));

            return;
        }
    }*/

    // Najlepiej bylo by wybrac najd³uzszy kierunek,
    // ale jesli jest ponizej pewnej odleglosci to wykonac unik w miejscu.
    float DirectionDistance = GetDirectionClearDistance(Direction, RightVector);
    float OppositeDirectionDistance = GetDirectionClearDistance(OppositeDirection, RightVector);

    float GreaterDirectionDistance = 0.0f;
    float GreaterDirection = 0.0f;

    if (DirectionDistance >= OppositeDirectionDistance)
    {
        GreaterDirectionDistance = DirectionDistance;
        GreaterDirection = Direction;
    }
    else
    {
        GreaterDirectionDistance = OppositeDirectionDistance;
        GreaterDirection = OppositeDirection;
    }

    //UE_LOG(LogTemp, Warning, TEXT("Direction Distance = %f, Opposite Direction Distance = %f, Greater Direction Distance = %f"), DirectionDistance, OppositeDirectionDistance, GreaterDirectionDistance);

    if (GreaterDirectionDistance < MaxDodgeInPlaceDistance)
    {
        EnemyCharacter->PlayAnimMontage(DodgeNoMoveMontage);

        UE_LOG(LogTemp, Warning, TEXT("Obstruction on dodge path! Dodging without move."));

        return;
    }


    EnemyCharacter->PlayAnimMontage((GreaterDirection > 0.0f) ? DodgeRightMontage : DodgeLeftMontage);

    EnemyCharacter->GetCharacterMovement()->BrakingFrictionFactor = 0;
    EnemyCharacter->GetCharacterMovement()->BrakingDecelerationWalking = 0;

    EnemyCharacter->LaunchCharacter(RightVector * GreaterDirection * DodgeSpeed, true, false);
}

void AEnemyController::DodgeEnd()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    EnemyCharacter->LaunchCharacter(FVector::ZeroVector, true, false);

    EnemyCharacter->GetCharacterMovement()->BrakingFrictionFactor = BrakingFrictionFactorSave;
    EnemyCharacter->GetCharacterMovement()->BrakingDecelerationWalking = BrakingDecelerationWalkingSave;

    if (DodgeCooldown > 0)
    {
        GetWorldTimerManager().SetTimer(DodgeCooldownTimer, DodgeCooldown, false);
    }

    if (CurrentAbility == nullptr)
    {
        GetWorldTimerManager().ClearTimer(DodgeTimer);
        ActivateNewAbility();
    }
}

//bool AEnemyController::IsDodgePathClear(float Direction, FVector RightVector)
//{
//    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
//
//    FVector Location = GetPawn()->GetActorLocation() + RightVector * Direction * DodgeDistance;
//    FNavLocation NavLoc;
//    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
//
//    if (!NavSystem->ProjectPointToNavigation(Location, NavLoc, FVector(MaxDistanceDifferenceForPathCheck)))
//    {
//        UE_LOG(LogTemp, Warning, TEXT("Project point to navigation failed!"));
//
//        return false;
//    }
//
//    FAIMoveRequest MoveReq(NavLoc);
//    FPathFindingQuery PFQuery;
//
//    if (!BuildPathfindingQuery(MoveReq, PFQuery))
//    {
//        UE_LOG(LogTemp, Warning, TEXT("Build pathfinding query failed!"));
//
//        return false;
//    }
//
//    FNavPathSharedPtr Path;
//    FindPathForMoveRequest(MoveReq, PFQuery, Path);
//
//    if (!Path.IsValid())
//    {
//        UE_LOG(LogTemp, Warning, TEXT("Path isn't valid!"));
//
//        return false;
//    }
//
//    float DistanceDifference = DodgeDistance - Path.Get()->GetLength();
//    DistanceDifference = FMath::Abs(DistanceDifference);
//
//    return DistanceDifference <= MaxDistanceDifferenceForPathCheck;
//}

float AEnemyController::GetDirectionClearDistance(float Direction, FVector RightVector)
{
    checkSlow(Direction != 0.0f);

    FVector TraceStart = EnemyCharacter->GetActorLocation();
    FVector TraceEnd = RightVector * Direction * DodgeDistance + TraceStart;

    UCapsuleComponent* EnemyCapsule = EnemyCharacter->GetCapsuleComponent();
    check(EnemyCapsule);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(EnemyCharacter);
    /*QueryParams.TraceTag = TEXT("TraceTag");
    GetWorld()->DebugDrawTraceTag = TEXT("TraceTag");*/

    FHitResult Hit;

    if (GetWorld()->SweepSingleByProfile(Hit, TraceStart, TraceEnd, EnemyCapsule->GetComponentQuat(), DodgeCollisionPreset, EnemyCapsule->GetCollisionShape(EnemyCapsuleInflation), QueryParams))
    {
        return Hit.Distance * Hit.Time;
    }

    return DodgeDistance;
}
#pragma endregion
