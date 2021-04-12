// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShakeBase;

/*
	define a structure that holds an information about hit trace
	having a start and end point of the trace

	FVector_NetQuantize - is a serialized vector format for the transferring over the network
*/
USTRUCT()
struct FHitScanTrace {
	
	GENERATED_BODY()

public:

	UPROPERTY()
	FVector_NetQuantize TraceStart;

	UPROPERTY()
	FVector_NetQuantize TraceEnd;

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface>	Surface;
};

UCLASS()
class COOPSHOOTER_API ASWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, Category = "MuzzleEffects")
		FName	MuzzleSocket;

	UPROPERTY(VisibleDefaultsOnly, Category = "MuzzleEffects")
		FName	TraceTargetParamName;

	UPROPERTY(EditDefaultsOnly, Category = "MuzzleEffects")
		TSubclassOf<UCameraShakeBase>	FireCameraShakeEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
		UParticleSystem* MuzzleFlashEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
		UParticleSystem* VulnureableSurfaceEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
		UParticleSystem* DefaultSurfaceEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
		UParticleSystem* TraceEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Damage", meta = (ClampMin = 10, ClampMax = 100))
		float WeaponDamage;

	void	PlayFireEffects(const FVector& TraceHitEnd);
	void	PlayImpactEffect(const EPhysicalSurface& Surface, const FVector& ImpactPoint);

	UPROPERTY(EditDefaultsOnly, Category = "Damage", meta = (ClampMin = 10, ClampMax = 1000))
	float BulletsPerMinute;

	float LastFireTime;
	float FireRate;

	FTimerHandle	FireTimer_Handler;

	virtual void	Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void			ServerFire();

	virtual void	BeginPlay();

	/*
		this method OnRep_HitScanTrace is called each time when the HitScanTrace is replicated
		so all the clients will recieve updated values
	*/
	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace	HitScanTrace;

	UFUNCTION()
	void	OnRep_HitScanTrace();

public:
	void	StartFire();

	void	EndFire();

};