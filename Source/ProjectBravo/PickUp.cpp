// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "Player/PlayerCharacter.h"

// Sets default values
APickUp::APickUp()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    RangeCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RangeCollision"));
    RangeCollision->SetupAttachment(Mesh);
}

// Called when the game starts or when spawned
void APickUp::BeginPlay()
{
    Super::BeginPlay();

    check(Type != None);

    RangeCollision->OnComponentBeginOverlap.AddDynamic(this, &APickUp::OnBeginOverlap);
}

// Called every frame
void APickUp::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APickUp::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);

    if (!PlayerCharacter)
    {
        return;
    }

    switch (Type)
    {
    case Weapon:
        PlayerCharacter->AddWeapon(WeaponTag);
        Destroy();
        break;

    case Ammo:
        PlayerCharacter->AddAmmo(WeaponTag, AmmoAmount);
        Destroy();
        break;

    case Health:
        if (PlayerCharacter->GetCurrentHealth() == PlayerCharacter->GetMaxHealth())
        {
            break;
        }

        PlayerCharacter->Heal(HealthAmount, ArmorAmount);
        Destroy();
        break;

    case Armor:
        if (PlayerCharacter->GetCurrentArmor() == PlayerCharacter->GetMaxArmor())
        {
            break;
        }

        PlayerCharacter->Heal(HealthAmount, ArmorAmount);
        Destroy();
        break;

    case RevolverCharge:
    {
        /*int32 MissingRevolverCharges = PlayerCharacter->GetMaxRevolverCharges() - PlayerCharacter->GetCurrentRevolverCharges();

        if (MissingRevolverCharges > 0)
        {
            for (int32 i = 1; i <= MissingRevolverCharges; i++)
            {
                PlayerCharacter->AddRevolverCharge();
            }
        }*/

        break;
    }
    default:
        break;
    }
}
