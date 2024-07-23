// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TP_HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPS_API UTP_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTP_HealthComponent();

	UPROPERTY(SaveGame, EditAnywhere, meta = (UIMin = 0, UIMax = 100), meta = (EditCondition = "bIsImmortal == false"))
	int Health = 100;

	UPROPERTY(EditAnywhere)
	bool bIsImmortal = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	int GetHelth() { return Health; }

	UFUNCTION(BlueprintCallable)
	void ApplyDamage(int damage);
		
};
