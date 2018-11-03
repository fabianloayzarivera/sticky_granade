// Copyright 1998-2017 Epic Games, Inc. All Rights Reerved.

#include "GranadeTestProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Stickable.h"
#include "Damageable.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

AGranadeTestProjectile::AGranadeTestProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AGranadeTestProjectile::OnHit);		// set up a notification for when this component hits something blocking

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

	ExplosionDelay = 2.0f;
	ExplosionDamage = 50.f;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	GranadeParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GranadeParticleSystemComponent"));
	GranadeParticleSystemComponent->SetupAttachment(RootComponent);

	RenderDebugInfo = false;
	// Die after 3 seconds by default
	//InitialLifeSpan = 3.0f;
}

void AGranadeTestProjectile::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(GranadeCountDownTimerHandler, this, &AGranadeTestProjectile::ActivateGranade, GranadeCountDown, false);

}


void AGranadeTestProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		//Destroy();
	}

	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) 
	{
		
		if (OtherActor->GetClass()->ImplementsInterface(UStickable::StaticClass())) 
		{
			IStickable* StickableActor = Cast<IStickable>(OtherActor);
			if (StickableActor)
			{
				StickableActor->OnStick();
				this->ProjectileMovement->StopMovementImmediately();
				this->CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				this->AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform);
				ActivateGranade();
				//UGameplayStatics::SpawnEmitterAttached(ActivatedParticleSystem, MeshComponent, MeshComponent, GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
			}
		}
		
	}
}


void AGranadeTestProjectile::Explode() 
{
	GetWorld()->GetTimerManager().ClearTimer(ExplosionTimerHandler);
	ExplosionTimerHandler.Invalidate();
	/*if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "EXPLODE!");*/

	FTransform ExplosionTransform;
	ExplosionTransform.SetLocation(this->GetActorLocation());
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticleSystem, ExplosionTransform);

	TArray<AActor*> HitResults;
	TArray<AActor*> ActorsToIgnore;
	TArray < TEnumAsByte < EObjectTypeQuery > > ObjectTypesOverlap;
	ObjectTypesOverlap.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	ObjectTypesOverlap.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));
	ObjectTypesOverlap.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	ActorsToIgnore.Add(GetWorld()->GetFirstPlayerController()->GetPawn());

	/*The Start location of the sphere*/
	FVector StartLocation = this->GetActorLocation();

	bool bHitSomething = UKismetSystemLibrary::SphereOverlapActors(this, StartLocation, ExplosionRadius, ObjectTypesOverlap, AActor::StaticClass(), ActorsToIgnore, HitResults);

	if (bHitSomething)
	{
		for (auto It = HitResults.CreateIterator(); It; It++)
		{
			/*if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, (*It)->GetName());*/

			FHitResult TraceResult;
			FVector Direction;
			FCollisionQueryParams TraceParams;
			TraceParams.bTraceAsyncScene = true;
			TraceParams.AddIgnoredActor(this);
			Direction = (*It)->GetActorLocation() - StartLocation;
			Direction.Normalize();
			Direction *= ExplosionRadius;
			GetWorld()->LineTraceSingleByChannel(TraceResult, StartLocation, StartLocation + Direction, ECC_Visibility, TraceParams);

			if (TraceResult.bBlockingHit)
			{
				if (RenderDebugInfo)
				{
					DrawDebugLine(GetWorld(), StartLocation, TraceResult.ImpactPoint, FColor::Green, false, 5.f);
					DrawDebugPoint(GetWorld(), TraceResult.ImpactPoint, 5.0f, FColor::Blue, false, 5.f);
				}
				if (TraceResult.Actor == (*It))
				{
					if (TraceResult.Component->IsSimulatingPhysics())
					{
						Direction.Normalize();
						Direction *= ExplosionForce;
						TraceResult.Component->AddImpulseAtLocation(Direction, TraceResult.ImpactPoint);
						
					}
					/*if (GEngine)
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, (*It)->GetName());*/

					IDamageable* DamageableActor = Cast<IDamageable>(*It);
					if (DamageableActor)
						DamageableActor->OnDamage(ExplosionDamage);
					
				}
			}
			else
			{
				if (RenderDebugInfo)
					DrawDebugLine(GetWorld(), StartLocation, TraceResult.ImpactPoint, FColor::Red, false, 5.f);
			}
		}
	}

	if (RenderDebugInfo)
	{
		DrawDebugSphere(GetWorld(), StartLocation, ExplosionRadius, 100.f, FColor::Red, false, 5.0f);
	}


	Destroy();
}

void AGranadeTestProjectile::ActivateGranade() 
{
	if (ActivatedParticleSystem)
	{
		GranadeParticleSystemComponent->SetTemplate(ActivatedParticleSystem);
		//GranadeParticleSystemComponent->ResetParticles();
	}
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, "Granade Activated!");
	GetWorld()->GetTimerManager().ClearTimer(GranadeCountDownTimerHandler);
	GranadeCountDownTimerHandler.Invalidate();
	GetWorld()->GetTimerManager().SetTimer(ExplosionTimerHandler, this, &AGranadeTestProjectile::Explode, ExplosionDelay, false);
}