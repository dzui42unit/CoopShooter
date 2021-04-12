// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Public/SWeapon.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COOPSHOOTER_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// move forward function
	void	MoveForward(float Value);
	// move sides function
	void	MoveSides(float Value);

	// camera component of the character
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;
	// spring arm component of the character
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;
	// health component
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;
	
	// begin crouch
	void	BeginCrouch();
	// end crouch
	void	EndCrouch();

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1f, ClampMax = 100.f))
	float	FovInterpSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float	FovZoomed;

	float	DefaultFov;

	bool	bWantsToZoom;

	void	BeginZoom();

	void	EndZoom();

	UPROPERTY(Replicated)
	ASWeapon* CurrentWeapon;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName	WeaponSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon>		WeaponType;

	void		StartFire();
	void		EndFire();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool		bDied;

	UFUNCTION()
	void	OnHealthChanged(USHealthComponent* Hc, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FVector GetPawnViewLocation() const override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
