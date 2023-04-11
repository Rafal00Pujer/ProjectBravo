// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameplayTagContainer.h"
#include "PickUp.generated.h"

UENUM()
enum EPickUpType
{
	Weapon,
	Ammo,
	Health,
	Armor,
	RevolverCharge,
	None
};

UCLASS()
class PROJECTBRAVO_API APickUp : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
		USphereComponent* RangeCollision;

	UFUNCTION()
		void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TEnumAsByte<EPickUpType> Type = None;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "Type == EPickUpType::Weapon || Type == EPickUpType::Ammo"))
		FGameplayTag WeaponTag;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", EditCondition = "Type == EPickUpType::Ammo"))
		int32 AmmoAmount = 0;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", EditCondition = "Type == EPickUpType::Health"))
		float HealthAmount = 0;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", EditCondition = "Type == EPickUpType::Armor"))
		float ArmorAmount = 0;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", EditCondition = "Type == EPickUpType::RevolverCharge"))
		int32 RevolverCharges = 0;
};
