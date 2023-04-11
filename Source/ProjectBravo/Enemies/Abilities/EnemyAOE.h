// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyAOE.generated.h"

class USphereComponent;

UCLASS()
class PROJECTBRAVO_API AEnemyAOE : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyAOE();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    const USphereComponent* GetAOECollision() const;

private:
    UPROPERTY(VisibleAnywhere)
        USphereComponent* AOECollision;

    UPROPERTY(VisibleAnywhere)
        USphereComponent* AOEDamageOverlap;
};
