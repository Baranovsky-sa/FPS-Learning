// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "FPSCharacter.h"
#include "FPSProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

static TAutoConsoleVariable<bool> CVarInfiniteAmmo(
	TEXT("u.InfiniteAmmo"),
	false,
	TEXT("Used to chets amount of ammo\n")
	TEXT("<=0: off \n")
	TEXT(" 1: infinite\n"),
	ECVF_Cheat);

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}


void UTP_WeaponComponent::Fire()
{
	const bool IsInfiniteCheatOn = CVarInfiniteAmmo.GetValueOnGameThread();

	if (Character == nullptr || Character->GetController() == nullptr || (Character->AmmoCount <= 0 && !IsInfiniteCheatOn))
	{
		return;
	}

	switch (FireMode)
	{
	case EFireMode::Projectile:
		FireProjectile();
		break;
	case EFireMode::HitScan:
		FireLaser();
		break;
	case EFireMode::GravityGun:
		FireGravityGun();
		break;
	case EFireMode::Burst:
		FireBurst();
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("FireMode out of enum %d"), FireMode);
		break;
	}
}

void UTP_WeaponComponent::SwitchFireMode()
{
	FireMode = static_cast<EFireMode>((static_cast<uint8>(FireMode) + 1) % static_cast<uint8>(EFireMode::Burst) + 1);
}

bool UTP_WeaponComponent::AttachWeapon(AFPSCharacter* TargetCharacter)
{
	Character = TargetCharacter;

	// Check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UTP_WeaponComponent>())
	{
		return false;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	// add the weapon as an instance component to the character
	Character->AddInstanceComponent(this);

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
			
			// FireMode
			EnhancedInputComponent->BindAction(SwitchFireModeAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::SwitchFireMode);
		}

	}

	return true;
}

void UTP_WeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GravityHoldActor && PhysicsHandle->GrabbedComponent)
	{
		const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
		const FRotator SpawnRotator = PlayerController->PlayerCameraManager->GetCameraRotation();
		const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotator.RotateVector(MuzzleOffset);
		const FVector End = SpawnLocation + PlayerController->PlayerCameraManager->GetActorForwardVector() * 300.f;

		PhysicsHandle->SetTargetLocationAndRotation(End, SpawnRotator);
	}
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}

void UTP_WeaponComponent::FireProjectile()
{
	const bool IsInfiniteCheatOn = CVarInfiniteAmmo.GetValueOnGameThread();

	// Try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// Spawn the projectile at the muzzle
			World->SpawnActor<AFPSProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

			if (!IsInfiniteCheatOn)
			{
				--Character->AmmoCount;
			}
		}
	}

	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void UTP_WeaponComponent::FireLaser()
{
	const FVector MuzzleLaserOffset = FVector(45.0f, 5.0f, 15.0f);
	const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	const FRotator PlayerCameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + PlayerCameraRotation.RotateVector(MuzzleLaserOffset);
	const FVector EndLocation = SpawnLocation + PlayerController->PlayerCameraManager->GetActorForwardVector() * FireRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, SpawnLocation, EndLocation, ECC_Visibility, Params))
	{
		DrawDebugLine(GetWorld(), SpawnLocation, Hit.ImpactPoint, FColor::Red, false, 1.f, 0.f, 1.f);
		if (AActor* HitActor = Hit.GetActor())
		{
			UGameplayStatics::ApplyDamage(HitActor, 25.f, nullptr, GetOwner(), nullptr);
		}
	}
	else
	{
		DrawDebugLine(GetWorld(), SpawnLocation, EndLocation, FColor::Blue, false, 1.0f, 0, 1.f);
	}
}

void UTP_WeaponComponent::FireGravityGun()
{
	const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	const FRotator SpawnRotator = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotator.RotateVector(MuzzleOffset);
	const FVector End = SpawnLocation + PlayerController->PlayerCameraManager->GetActorForwardVector() * 500.f;

	if (!GravityHoldActor)
	{
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());

		if (GetWorld()->LineTraceSingleByChannel(Hit, SpawnLocation, End, ECC_PhysicsBody, Params))
		{
			UPrimitiveComponent* HitComp = Hit.GetComponent();
			if (HitComp && HitComp->IsSimulatingPhysics())
			{
				UE_LOG(LogTemp, Warning, TEXT("Gravity taken"));
				GravityHoldActor = Hit.GetActor();
				PhysicsHandle->GrabComponentAtLocationWithRotation(HitComp, NAME_None, Hit.ImpactPoint, HitComp->GetComponentRotation());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Released"));
		PhysicsHandle->ReleaseComponent();
		GravityHoldActor = nullptr;
	}
}

void UTP_WeaponComponent::FireBurst()
{
}
