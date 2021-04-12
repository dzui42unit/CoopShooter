// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Camera/CameraShake.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../CoopShooter.h"
#include "Net/UnrealNetwork.h"

static int32 DrawWeaponDebug = 0;
FAutoConsoleVariableRef CVARDrawWeaponDebug(
	TEXT("COOP.DebugWeapon"),
	DrawWeaponDebug,
	TEXT("Draw debug lines for weapons"),
	ECVF_Cheat
);

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// create a mesh for the weapon
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// muzzle flash socket
	MuzzleSocket = "MuzzleFlashSocket";
	// trace target parameter name
	TraceTargetParamName = "BeamEnd";

	WeaponDamage = 20.f;

	BulletsPerMinute = 100.f;

	SetReplicates(true);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASWeapon::PlayFireEffects(const FVector& TraceHitEnd)
{
	if (MuzzleFlashEffect) {
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlashEffect, MeshComp, MuzzleSocket);
	}

	if (TraceEffect) {
		FVector TraceStart = MeshComp->GetSocketLocation(MuzzleSocket);
		UParticleSystemComponent* TraceParticleComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TraceEffect, TraceStart);
		if (TraceParticleComp) {
			TraceParticleComp->SetVectorParameter(TraceTargetParamName, TraceHitEnd);
		}
	}

	APawn* Onwer = Cast<APawn>(GetOwner());
	APlayerController* PC = Cast<APlayerController>(Onwer->GetController());

	if (PC) {
		PC->ClientStartCameraShake(FireCameraShakeEffect);
	}
}

void ASWeapon::PlayImpactEffect(const EPhysicalSurface& Surface, const FVector& ImpactPoint)
{
	UParticleSystem* SurfaceEffect = nullptr;

	switch (Surface)
	{
	case EPhysicalSurface::SurfaceType1:
	case EPhysicalSurface::SurfaceType2:
		SurfaceEffect = VulnureableSurfaceEffect;
		break;
	default:
		SurfaceEffect = DefaultSurfaceEffect;
		break;
	}

	FVector ShotStart = MeshComp->GetSocketLocation(MuzzleSocket);
	FVector ShotEnd = ImpactPoint - ShotStart;
	ShotEnd.Normalize();

	if (SurfaceEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SurfaceEffect, ImpactPoint, ShotEnd.Rotation());
	}
}

void	ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	FireRate = 60 / BulletsPerMinute;
}

void ASWeapon::StartFire()
{
	float FireDelay = FMath::Max(LastFireTime + FireRate - GetWorld()->GetTimeSeconds(), 0.f);

	GetWorldTimerManager().SetTimer(FireTimer_Handler, this, &ASWeapon::Fire, FireRate, true, FireDelay);
}

void ASWeapon::EndFire()
{
	GetWorldTimerManager().ClearTimer(FireTimer_Handler);
}

void ASWeapon::Fire()
{
	AActor* WeaponOwner = GetOwner();

	/*	
		if the fire event was triggered on the client -> send a request to the server to run fire functionality and simply return
		from function, so only the server runs fire functionality
		if (!HasAuthority()) {
			ServerFire();
			return;
		}
	*/

	/*
		It runs now both on the server and a client side, but client does not see the effects on caused on the server side
	*/
	if (!HasAuthority()) {
		ServerFire();
	}

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

		// ignore the gun actor and its owner, make it return a physical material
		HitParams.AddIgnoredActor(this);
		HitParams.AddIgnoredActor(WeaponOwner);
		HitParams.bReturnPhysicalMaterial = true;

		// define a physical suraface as a default
		EPhysicalSurface PhysSurface = SurfaceType_Default;

		GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, WAEPON_COLLISION, HitParams);

		if (Hit.bBlockingHit) {
			AActor* HitActor =  Hit.GetActor();

			// get the hit point and physical marerial that was hit
			TraceHitEnd = Hit.ImpactPoint;
			PhysSurface =  UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			PlayImpactEffect(PhysSurface, TraceHitEnd);

			// make a damage to the head bigger 10 times
			float DamageToDeal = WeaponDamage;
			if (PhysSurface == EPhysicalSurface::SurfaceType2) {
				DamageToDeal = DamageToDeal * 10;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, DamageToDeal, HitDirection, Hit, WeaponOwner->GetInstigatorController(), this, DamageType);
		}

		PlayFireEffects(TraceHitEnd);

		/*
			update the HitScanTrace by setting the TraceEnd value and send it to all clients
		*/
		if (HasAuthority()) {
			HitScanTrace.TraceEnd = TraceHitEnd;
			HitScanTrace.Surface = PhysSurface;
			HitScanTrace.TraceStart = TraceHitEnd;
		}

		if (DrawWeaponDebug) {
			DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, 3.f, 0, 2.f);
		}

		LastFireTime = GetWorld()->GetTimeSeconds();
	}
}


void ASWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(HitScanTrace.TraceEnd);
	PlayImpactEffect(HitScanTrace.Surface, HitScanTrace.TraceStart);
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
