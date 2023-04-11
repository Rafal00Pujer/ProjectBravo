// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectBravoGameMode.h"
#include "EngineUtils.h"

void AProjectBravoGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AEnemyCharacter> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		SpawnedEnemies.Add(*Iterator);
		Iterator->OnDestroyed.AddDynamic(this, &AProjectBravoGameMode::OnEnemyDestroy);
	}
}

const TSet<AEnemyCharacter*>& AProjectBravoGameMode::GetSpawnedEnemies()
{
	return SpawnedEnemies;
}

void AProjectBravoGameMode::OnEnemyDestroy(AActor* DestroyedActor)
{
	SpawnedEnemies.Remove(Cast<AEnemyCharacter>(DestroyedActor));
}
