// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AExplosive::AExplosive()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult HitResult)
{
	if (ExplodeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	}
	if (ExplodeParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParticles, HitResult.Location, FRotator(0.f, 0.0f, 0.0f), true);
	}

	// TODO: 应用爆炸上海

	Destroy();
}

