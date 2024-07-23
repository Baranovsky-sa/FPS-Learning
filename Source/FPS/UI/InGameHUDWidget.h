// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API UInGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	void UpdateAmmoCount(int ammo);

	UPROPERTY(EditAnywhere)
	int AmmoCount = 0;
	
};
