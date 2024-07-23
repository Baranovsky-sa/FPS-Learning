// Fill out your copyright notice in the Description page of Project Settings.


#include "TP_RadialForceComponent.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UTP_RadialForceComponent::UTP_RadialForceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UTP_RadialForceComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UTP_RadialForceComponent::ApplyRadialForce(float Radius, float Strength)
{
	if (!GetOwner())
		return;

	FVector Origin = GetOwner()->GetActorLocation();

	TArray<AActor*> OverlappingActors;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == GetOwner())
			continue;

		float Distance = FVector::Dist(Origin, Actor->GetActorLocation());
		if (Distance > Radius)
			continue;

		FHitResult Hit;
		TArray<FHitResult> HitResults;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(GetOwner());

		FVector ActorLocation = Actor->GetActorLocation();

		GetWorld()->SweepMultiByChannel(HitResults, Origin, Origin, FQuat::Identity, ECC_PhysicsBody,Sphere,TraceParams);

		for (auto hitR : HitResults)
		{
			TraceParams.AddIgnoredActor(hitR.GetActor());
			auto hitActorLocation = hitR.GetActor()->GetActorLocation();
			bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, Origin, hitActorLocation, ECC_Visibility, TraceParams);
			
			DrawDebugSphere(GetWorld(), Origin, Radius,10, FColor::Blue, false, 1.0, 0, 2.0f);
			
			if (!bBlocked)
			{
				DrawDebugLine(GetWorld(), Origin, hitR.GetActor()->GetActorLocation(), FColor::Green, false, 1.0, 0, 2.0f);
				ApplyForceToActor(hitR.GetActor(), Strength);
			}
			else
			{
				DrawDebugLine(GetWorld(), Origin, hitR.GetActor()->GetActorLocation(), FColor::Red, false, 1.0, 0, 2.0f);
			}
		}
		//bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, Origin, ActorLocation, ECC_Visibility, TraceParams);

		//

		//if (!bBlocked)
		//{
		//	ApplyForceToActor(Actor, Strength);
		//}

	}

}

void UTP_RadialForceComponent::ApplyForceToActor(AActor* Actor, float Strength)
{
	if (UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
	{
		if (RootComponent->IsSimulatingPhysics())
		{
			FVector Direction = (Actor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
			RootComponent->AddImpulse(Direction * Strength, NAME_None, true);
		}
	}
}

