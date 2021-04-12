// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBarrel.generated.h"

class UStaticMeshComponentl;
class USHealthComponent;
class UParticleSystem;
class URadialForceComponent;

UCLASS()
class COOPSHOOTER_API ASBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Components")
	UStaticMeshComponent*	MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USHealthComponent*		HealthComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UMaterial*				BarrelExplodedMaterial;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent*	ForceComp;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem*		BarrelsExplodeEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float					ExplodeHeight;

	UPROPERTY(ReplicatedUsing = OnRep_Explode)
	bool					bExploded;

	UFUNCTION()
	void	OnRep_Explode();

	void	ChangeBarrelMaterial();

	UFUNCTION()
	void	OnHealthChanged(USHealthComponent* Hc, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};
