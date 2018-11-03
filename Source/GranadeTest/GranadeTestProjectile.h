// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "GranadeTestProjectile.generated.h"

UCLASS(config=Game)
class AGranadeTestProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		class UProjectileMovementComponent* ProjectileMovement;
	

public:

	AGranadeTestProjectile();

	FTimerHandle ExplosionTimerHandler;

	FTimerHandle GranadeCountDownTimerHandler;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
		UStaticMeshComponent* MeshComponent; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		UParticleSystem* ExplosionParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		UParticleSystemComponent* GranadeParticleSystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		UParticleSystem* ActivatedParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		float ExplosionDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		float GranadeCountDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		float ExplosionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		float ExplosionForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		float ExplosionDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		bool RenderDebugInfo;

	/** called when projectile hits something */
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void ActivateGranade();

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
protected:
	virtual void BeginPlay() override;

	void Explode();
};

