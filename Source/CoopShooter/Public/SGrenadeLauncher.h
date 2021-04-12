// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGrenadeLauncher.generated.h"

class AProjectile;

/**
 * 
 */
UCLASS()
class COOPSHOOTER_API ASGrenadeLauncher : public ASWeapon
{
	GENERATED_BODY()

	ASGrenadeLauncher();
protected:
	virtual void	Fire() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AProjectile>	Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	float						ProjectileImpulse;

	void		ExplodeProjectile(AProjectile* CreatedPojectile);
};
