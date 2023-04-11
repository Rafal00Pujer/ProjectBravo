// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "ProjectBravo/Player/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnemyController.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    FirstPickUpSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("FirstPickUpSpawn"));
    FirstPickUpSpawn->SetupAttachment(RootComponent);

    SecondPickUpSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("SecondPickUpSpawn"));
    SecondPickUpSpawn->SetupAttachment(RootComponent);

    ThirdPickUpSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("ThirdPickUpSpawn"));
    ThirdPickUpSpawn->SetupAttachment(RootComponent);

    FourthPickUpSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("FourthPickUpSpawn"));
    FourthPickUpSpawn->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    IncendaryGrenadeTick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

#pragma region Pick Ups Spawn
void AEnemyCharacter::SpawnRandomPickUP(const TArray<TSubclassOf<APickUp>>& PickUpsClass)
{
    check(PickUpsClass.Num() > 0);
    check(FirstPickUpSpawn != nullptr);
    check(SecondPickUpSpawn != nullptr);
    check(ThirdPickUpSpawn != nullptr);
    check(FourthPickUpSpawn != nullptr);

    int32 RandomIndex = FMath::RandRange(0, PickUpsClass.Num() - 1);
    UClass* PickUpClass = PickUpsClass[RandomIndex];

    int32 RandomPickUpSpawn = FMath::RandRange(1, 4);
    USceneComponent* RandomPickUpSpawnPoint = nullptr;

    if (RandomPickUpSpawn == 1)
    {
        RandomPickUpSpawnPoint = FirstPickUpSpawn;
    }
    else if (RandomPickUpSpawn == 2)
    {
        RandomPickUpSpawnPoint = SecondPickUpSpawn;
    }
    else if (RandomPickUpSpawn == 3)
    {
        RandomPickUpSpawnPoint = ThirdPickUpSpawn;
    }
    else if (RandomPickUpSpawn == 4)
    {
        RandomPickUpSpawnPoint = FourthPickUpSpawn;
    }
    else
    {
        checkNoEntry();
    }

    FVector RandomImpuls;
    RandomImpuls.X = FMath::RandRange(MinPickUpSpawnImpuls.X, MaxPickUpSpawnImpuls.X);
    RandomImpuls.Y = FMath::RandRange(MinPickUpSpawnImpuls.Y, MaxPickUpSpawnImpuls.Y);
    RandomImpuls.Z = FMath::RandRange(MinPickUpSpawnImpuls.Z, MaxPickUpSpawnImpuls.Z);

    // Try deferred spawn actor.
    APickUp* SpawnedPickUp = GetWorld()->SpawnActor<APickUp>(PickUpClass, RandomPickUpSpawnPoint->GetComponentTransform());

    /*checkf(SpawnedPickUp, TEXT("Error spawning Pick Up."));*/

    if (SpawnedPickUp == nullptr)
    {
        /*UE_LOG(LogTemp, Error, TEXT("Spawned Pick Up is null!!! But why???? Because the Player is within Pick Up range and picks it up immediately."));*/
        return;
    }

    UPrimitiveComponent* SpawnedPickUpPrim = Cast<UPrimitiveComponent>(SpawnedPickUp->GetRootComponent());
    check(SpawnedPickUpPrim != nullptr);

    SpawnedPickUpPrim->AddImpulse(RandomImpuls);
}

void AEnemyCharacter::SpawnPlayerCriticalHealthPickUp()
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    check(PlayerController != nullptr);
    APawn* PlayerPawn = PlayerController->GetPawn();
    check(PlayerPawn != nullptr);
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PlayerPawn);
    check(PlayerCharacter);

    if (!PlayerCharacter->HasCriticalHealth())
    {
        return;
    }

    SpawnRandomPickUP(PlayerCriticalHealthPickUpsClass);
}
#pragma endregion

#pragma region Health
void AEnemyCharacter::AddDamage(FGameplayTag GrenadeTag, FGameplayTag WeaponTag, float DamageScale, const TArray<TSubclassOf<APickUp>>& PickUpsClass)
{
    /*UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));*/

    check(DamageTable.Contains(WeaponTag));
    checkf(DamageScale >= 0, TEXT("DamageScale is lower than 0."));

    if (GetWorldTimerManager().IsTimerActive(CorrosiveTimer))
    {
        DamageScale *= CorrosiveDamageMultiply;
    }

    float Damage = DamageScale * DamageTable[WeaponTag];

    if (Damage <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Skipping damage. Damage = %f."), Damage);
        return;
    }

    if (bIsImmuneToExplosive && (GrenadeTag == StandardGrenadeTag || GrenadeTag == CorrosiveTag || GrenadeTag == ElectricTag || GrenadeTag == IncendaryTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("Skipping damage. Immune to explosive."), Damage);
        return;
    }

    AEnemyController* EnemyController = GetController<AEnemyController>();
    check(EnemyController);

    if (EnemyController->IsDodging())
    {
        UE_LOG(LogTemp, Warning, TEXT("Skipping damage. Enemy is Dodging."));
        return;
    }

    CurrentHealth -= Damage;

    OnEnemyAddDamage.ExecuteIfBound(GrenadeTag, WeaponTag, DamageScale, Damage);

    UE_LOG(LogTemp, Warning, TEXT("Weapon Tag: %s, Damage Scale: %f, Health: %f."), *WeaponTag.ToString(), DamageScale, CurrentHealth);

    FTimerManager& TimerManager = GetWorldTimerManager();

    if (CurrentHealth <= 0 && !TimerManager.IsTimerActive(DeathTimer))
    {
        CurrentHealth = 0;

        UE_LOG(LogTemp, Warning, TEXT("Enemy died!!!"));

        check(DeathMontage);

        DetachFromControllerPendingDestroy();

        GetCharacterMovement()->DisableMovement();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PlayAnimMontage(DeathMontage);

        TimerManager.SetTimer(DeathTimer, this, &AEnemyCharacter::DeathMontageEnd, DeathMontage->GetPlayLength() - 2.0f * GetWorld()->GetDeltaSeconds());

        if (PickUpsClass.Num() > 0)
        {
            SpawnRandomPickUP(PickUpsClass);
        }

        SpawnPlayerCriticalHealthPickUp();

        /*GetMesh()->SetSimulatePhysics(true);*/

        /*Destroy();*/
    }
    else
    {
        if (CorrosiveTag == GrenadeTag)
        {
            StartCorrosiveGrenade();
        }
        else if (ElectricTag == GrenadeTag)
        {
            StartElectricGrenade();
        }
        else if (IncendaryTag == GrenadeTag)
        {
            StartIncendaryGrenade();
        }
    }
}

float AEnemyCharacter::GetCurrentHealth()
{
    return CurrentHealth;
}

bool AEnemyCharacter::IsAlive()
{
    return CurrentHealth > 0.0f;
}

void AEnemyCharacter::AddDamage(float Damage)
{
    CurrentHealth -= Damage;

    if (CurrentHealth <= 0)
    {
        CurrentHealth = 0;

        UE_LOG(LogTemp, Warning, TEXT("Enemy died!!!"));

        check(DeathMontage);
        FTimerManager& TimerManager = GetWorldTimerManager();

        if (!TimerManager.IsTimerActive(DeathTimer))
        {
            GetCharacterMovement()->DisableMovement();
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            PlayAnimMontage(DeathMontage);

            TimerManager.SetTimer(DeathTimer, this, &AEnemyCharacter::DeathMontageEnd, DeathMontage->GetPlayLength() - 2.0f * GetWorld()->GetDeltaSeconds());
        }

        /*GetMesh()->SetSimulatePhysics(true);*/

        /*Destroy();*/
    }
}

void AEnemyCharacter::DeathMontageEnd()
{
    Destroy();
}
#pragma endregion

#pragma region Player Grenade Effect Corrosive
void AEnemyCharacter::StartCorrosiveGrenade()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    GetWorldTimerManager().SetTimer(CorrosiveTimer, CorrosiveGrenadeTime, false);
}
#pragma endregion

#pragma region Player Grenade Effect Electric
void AEnemyCharacter::StartElectricGrenade()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    if (!GetWorldTimerManager().IsTimerActive(ElectricTimer))
    {
        MovementSpeedSave = GetCharacterMovement()->MaxWalkSpeed;
        GetCharacterMovement()->MaxWalkSpeed = MovementSpeedWhileOnEffect;
    }

    GetWorldTimerManager().SetTimer(ElectricTimer, this, &AEnemyCharacter::EndElectricGrenade, ElectricGrenadeTime);
}

void AEnemyCharacter::EndElectricGrenade()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    GetCharacterMovement()->MaxWalkSpeed = MovementSpeedSave;
}
#pragma endregion

#pragma region Player Grenade Effect Incendary
void AEnemyCharacter::StartIncendaryGrenade()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    GetWorldTimerManager().SetTimer(IncendaryTimer, IncendaryGrenadeTime, false);
}

void AEnemyCharacter::IncendaryGrenadeTick(float DeltaTime)
{
    if (!GetWorldTimerManager().IsTimerActive(IncendaryTimer))
    {
        return;
    }

    AddDamage(IncendaryDamage * DeltaTime);
}
#pragma endregion