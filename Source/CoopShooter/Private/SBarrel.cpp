// Fill out your copyright notice in the Description page of Project Settings.


#include "SBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "../Public/Components/SHealthComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASBarrel::ASBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("Health Comp"));

	BarrelExplodedMaterial = CreateDefaultSubobject<UMaterial>(TEXT("Exploded Barrel material"));

	ForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("Radial Force Comp"));

	bExploded = false;
	ExplodeHeight = 20.f;

	SetReplicates(true);
	SetReplicatedMovement(true);
}

// Called when the game starts or when spawned
void ASBarrel::BeginPlay()
{
	Super::BeginPlay();

	HealthComp->OnHealthChanged.AddDynamic(this, &ASBarrel::OnHealthChanged);
}

void ASBarrel::OnRep_Explode()
{
	ChangeBarrelMaterial();
}

void ASBarrel::ChangeBarrelMaterial()
{
	if (MeshComp && BarrelExplodedMaterial) {
		MeshComp->SetMaterial(0, BarrelExplodedMaterial);
	}
}

void ASBarrel::OnHealthChanged(USHealthComponent* Hc, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded)
	{
		return;
	}

	if (Health <= 0.f) {
		UE_LOG(LogTemp, Warning, TEXT("DEAD"))
		bExploded = true;

		if (BarrelsExplodeEffect) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BarrelsExplodeEffect, GetActorLocation(), FRotator::ZeroRotator);
		}

		FVector ExplodeImpulse = FVector::UpVector * ExplodeHeight;
		MeshComp->AddImpulse(GetActorLocation() * ExplodeImpulse, NAME_None, true);

		ChangeBarrelMaterial();

		if (ForceComp) {
			ForceComp->FireImpulse();
		}
	}
}

void ASBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASBarrel, bExploded);
}
