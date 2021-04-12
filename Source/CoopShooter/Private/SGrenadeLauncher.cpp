// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenadeLauncher.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "../Public/Projectile.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"

ASGrenadeLauncher::ASGrenadeLauncher()
{
	ProjectileImpulse = 200.f;
}

DECLARE_DELEGATE_OneParam(ExplodeProjectileDelegate, AProjectile*)

void ASGrenadeLauncher::Fire()
{
	AActor* WeaponOwner = GetOwner();

	if (WeaponOwner) {

		FVector StartTrace;
		FRotator TraceRotation;

		// get starting vector and rotation
		WeaponOwner->GetActorEyesViewPoint(StartTrace, TraceRotation);

		// calculate end of the trace
		FVector HitDirection = TraceRotation.Vector();
		FVector EndTrace = HitDirection * 10000.f;

		FVector TraceHitEnd = EndTrace;

		FHitResult Hit;
		FCollisionQueryParams HitParams;

		HitParams.AddIgnoredActor(this);
		HitParams.AddIgnoredActor(WeaponOwner);

		//GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECollisionChannel::ECC_Visibility, HitParams);

		// spawn particles
		if (MuzzleFlashEffect) {
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlashEffect, MeshComp, MuzzleSocket);
		}

		StartTrace = MeshComp->GetSocketLocation(MuzzleSocket);;

		if (Projectile) {

			AProjectile* CreatedPojectile = GetWorld()->SpawnActor<AProjectile>(Projectile, StartTrace, TraceRotation);
			UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(CreatedPojectile->GetRootComponent());
			FVector LaunchForceVector = CreatedPojectile->GetActorRotation().Vector() * ProjectileImpulse;

			if (SM) {
				SM->AddImpulse(LaunchForceVector, NAME_None, true);

				FTimerHandle TimerExplodeProjectileHandler;
				FTimerDelegate ExplodeProjectileDelegate = FTimerDelegate::CreateUObject(this, &ASGrenadeLauncher::ExplodeProjectile, CreatedPojectile);
				GetWorldTimerManager().SetTimer(TimerExplodeProjectileHandler, ExplodeProjectileDelegate, 1.f, false);
			}
		}
	}
}

void ASGrenadeLauncher::ExplodeProjectile(AProjectile* CreatedPojectile)
{
	if (CreatedPojectile) {

		FVector DamageLocation = CreatedPojectile->GetActorLocation();
		AActor* WeaponOwner = GetOwner();
		TArray<AActor*> IgnoreActors = { this, WeaponOwner };
		FVector ProjectileLocation = CreatedPojectile->GetActorLocation();
		FRotator ProjectileRotation = ProjectileLocation.Rotation();

		UGameplayStatics::ApplyRadialDamage(GetWorld(), 25.f, DamageLocation, 200.f, DamageType, IgnoreActors, this, WeaponOwner->GetInstigatorController());
	
		if (DefaultSurfaceEffect) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DefaultSurfaceEffect, ProjectileLocation, ProjectileRotation);
		}

		CreatedPojectile->Destroy();
	}
}