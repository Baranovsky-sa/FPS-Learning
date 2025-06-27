// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSCharacter.h"
#include "FPSProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MySaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TP_HealthComponent.h"
#include "TP_WeaponComponent.h"
#include "EngineUtils.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "TableRows/WeaponRow.h"
#include "TP_PickUpComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AFPSCharacter

AFPSCharacter::AFPSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	HealthComponent = CreateDefaultSubobject<UTP_HealthComponent>(TEXT("HealthComponent"));

}

void AFPSCharacter::EquipLoadout(FName LoadoutName)
{
	const FString Context = TEXT("Loadout Lookup");

	if (WeaponTable == nullptr || LoadoutTable == nullptr)
		return;
	
	const FLoadoutRow* Loadout = LoadoutTable->FindRow<FLoadoutRow>(LoadoutName, Context);

	if (!Loadout) return;

	const FWeaponRow* primaryWeapon = WeaponTable->FindRow<FWeaponRow>(Loadout->PrimaryWeaponRow, Context);

	if (!primaryWeapon) return;

	AActor* SpawnedPrimary = GetWorld()->SpawnActor<AActor>(primaryWeapon->WeaponClass);

	UTP_PickUpComponent* PickupComponent = SpawnedPrimary->GetComponentByClass<UTP_PickUpComponent>();
	if (PickupComponent)
	{
		PickupComponent->OnPickUpByCharacter(this);
	}

	AmmoCount = Loadout->StartingAmmo;

}

void AFPSCharacter::NoClip()
{
	if (bIsNoclipEnabled)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		bIsNoclipEnabled = false;
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCharacterMovement()->BrakingDecelerationFlying = 10000.f;
		bIsNoclipEnabled = true;
	}
}

void AFPSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////// Input

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AFPSCharacter::AddAmmo(int ammo)
{
	AmmoCount += ammo;
}

void AFPSCharacter::SaveGame()
{
	UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));

	for (AActor* Actor : TActorRange<AActor>(GetWorld()))
	{
		FActorSaveData Data;
		Data.ActorName = Actor->GetFName().ToString();
		Data.ActorClass = Actor->GetClass();
		Data.Transform = Actor->GetTransform();

		FMemoryWriter MemWriter(Data.ByteData, true);
		FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
		Ar.ArIsSaveGame = true;
		Actor->Serialize(Ar);

		SaveGameInstance->SavedActors.Add(MoveTemp(Data));
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("MyGameSlot"), 0);
}

void AFPSCharacter::LoadGame()
{
	UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("MyGameSlot"), 0));

	for (FActorSaveData& data : SaveGameInstance->SavedActors)
	{
		AActor* Actor = nullptr;

		for (TActorIterator<AActor> It(GetWorld(), data.ActorClass); It; ++It)
		{
			if (It->GetFName().ToString() == data.ActorName)
			{
				Actor = *It;
				break;
			}
		}

		if (Actor == nullptr)
		{
			FActorSpawnParameters Params;
			Params.Name = *data.ActorName;
			Actor = GetWorld()->SpawnActor<AActor>(data.ActorClass, data.Transform, Params);
		}

		FMemoryReader MemReader(data.ByteData, true);
		FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
		Ar.ArIsSaveGame = true;
		Actor->Serialize(Ar);
		Actor->SetActorTransform(data.Transform);
	}
}

void AFPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (bIsNoclipEnabled)
	{
		AddMovementInput(GetFirstPersonCameraComponent()->GetForwardVector(), MovementVector.Y);
		AddMovementInput(GetFirstPersonCameraComponent()->GetRightVector(), MovementVector.X);
	}
	else
	{
		if (Controller != nullptr)
		{
			// add movement 
			AddMovementInput(GetActorForwardVector(), MovementVector.Y);
			AddMovementInput(GetActorRightVector(), MovementVector.X);
		}
	}
}

void AFPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Look X: %f, Y: %f"), LookAxisVector.X, LookAxisVector.Y);
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}