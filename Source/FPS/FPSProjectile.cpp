// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TP_HealthComponent.h"

AFPSProjectile::AFPSProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPSProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AFPSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && (OtherComp->IsSimulatingPhysics() || OtherActor->GetComponentByClass<UTP_HealthComponent>()))
	{
		if(OtherComp->IsSimulatingPhysics())
			OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		auto HealthCmp = OtherActor->GetComponentByClass<UTP_HealthComponent>();
		if (HealthCmp)
		{
			HealthCmp->ApplyDamage(10);
		}
		Destroy();
	}

	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 12.0f, 12, FColor::Red, false, 3.f);

	FVector End = Hit.ImpactPoint + Hit.ImpactNormal * 100;
	DrawDebugDirectionalArrow(GetWorld(), Hit.ImpactPoint, End, 60.f, FColor::Yellow, false, 3.f, 0, 2.f);

	DrawDebugString(GetWorld(), Hit.ImpactPoint + FVector(0, 0, 20), TEXT("HIT"), nullptr, FColor::White, 3.f, false);
}