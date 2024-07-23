// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MySaveGame.generated.h"

USTRUCT(BlueprintType)
struct FActorSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FString ActorName;

	UPROPERTY()
	TSubclassOf<AActor> ActorClass;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TArray<uint8> ByteData;
};
/**
 * 
 */
UCLASS()
class FPS_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FVector PlayerLocation;	
	
	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FRotator PlayerRotation;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	int PlayerAmmo;	
	
	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	int PlayerHealth;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	bool WeaponAttached;

	UPROPERTY()
	TArray<FActorSaveData> SavedActors;
};
