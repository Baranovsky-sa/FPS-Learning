// Fill out your copyright notice in the Description page of Project Settings.


#include "TP_HealthComponent.h"

// Sets default values for this component's properties
UTP_HealthComponent::UTP_HealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTP_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTP_HealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTP_HealthComponent::ApplyDamage(int damage)
{
	if(!bIsImmortal)
	{ 
		if (Health - damage > 0)
		{
			Health -= damage;
		}
		else
		{
			GetOwner()->Destroy();
		}
	}
}

