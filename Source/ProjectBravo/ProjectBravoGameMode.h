// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectBravoGameMode.generated.h"

/**
 *
 */
UCLASS()
class PROJECTBRAVO_API AProjectBravoGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	FORCEINLINE const TSet<AEnemyCharacter*>& GetSpawnedEnemies();

private:
	UPROPERTY(VisibleInstanceOnly)
		TSet<AEnemyCharacter*> SpawnedEnemies;

	UFUNCTION()
		void OnEnemyDestroy(AActor* DestroyedActor);
};
