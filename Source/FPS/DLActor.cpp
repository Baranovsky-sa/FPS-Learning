// Fill out your copyright notice in the Description page of Project Settings.


#include "DLActor.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

// Sets default values
ADLActor::ADLActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADLActor::BeginPlay()
{
	Super::BeginPlay();

	LoadMesh();
	
}

// Called every frame
void ADLActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADLActor::LoadMesh()
{
	if (MeshToLoad.IsNull() && MeshPath.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh is not set"));
		return;
	}

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

	if (!MeshToLoad.IsNull())
	{
		Streamable.RequestAsyncLoad(MeshToLoad.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &ADLActor::OnMeshLoaded));
	}
	else
	{
		Streamable.RequestAsyncLoad(MeshPath, FStreamableDelegate::CreateUObject(this, &ADLActor::OnMeshLoadedByPath));
	}
}

void ADLActor::OnMeshLoaded()
{
	UStaticMesh* Mesh = MeshToLoad.Get();
	if (Mesh)
	{
		UE_LOG(LogTemp, Log, TEXT("Dynamic mesh loaded: %s"), *Mesh->GetName());

		if (UStaticMeshComponent* SMComp = FindComponentByClass<UStaticMeshComponent>())
		{
			SMComp->SetStaticMesh(Mesh);
		}
	}
}

void ADLActor::OnMeshLoadedByPath()
{
	UStaticMesh* Mesh = Cast<UStaticMesh>(MeshPath.TryLoad());
	if (Mesh)
	{
		UE_LOG(LogTemp, Log, TEXT("Dynamic mesh loaded: %s"), *Mesh->GetName());

		if (UStaticMeshComponent* SMComp = FindComponentByClass<UStaticMeshComponent>())
		{
			SMComp->SetStaticMesh(Mesh);
		}
	}
}

