// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DLActor.generated.h"

UCLASS()
class FPS_API ADLActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADLActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void LoadMesh();

	void OnMeshLoaded();

	void OnMeshLoadedByPath();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category="Asset")
	TSoftObjectPtr<UStaticMesh> MeshToLoad;

	UPROPERTY(EditAnywhere, Category="Asset")
	FSoftObjectPath MeshPath;

};
