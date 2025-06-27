// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponRow.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FWeaponRow : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName WeaponName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FireRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> WeaponClass; // e.g., AMyRifleActor
};

USTRUCT(BlueprintType)
struct FLoadoutRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName PrimaryWeaponRow;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName SecondaryWeaponRow;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 StartingAmmo;
};