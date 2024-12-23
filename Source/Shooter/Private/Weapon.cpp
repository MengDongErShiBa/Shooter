// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
	ThrowWeaponTime(0.7),
	bFalling(false),
	Ammo(0),
	MagazineCapacity(30),
	WeaponType(EWeaponType::EWT_SubmachineGun),
	AmmoType(EAmmoType::EAT_9mm),
	ReloadMontageSection(FName(TEXT("ReloadSMG"))),
	ClipBoneName(TEXT("smg_clip"))
{
	// 开启Tick
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetItemState() == EItemState::EIS_Falling && bFalling)
	{
		// 保持武器直立
		const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
		// 设置世界旋转
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ThrowWeapon()
{
	// 获取武器朝向
	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
	// 设置世界旋转 何处理物理状态。TeleportPhysics 表示在设置位置或旋转时，物理引擎将不会进行插值，而是直接将物体移动到新位置或旋转到新角度。
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
	FVector MeshRight{ GetItemMesh()->GetRightVector() };
	// 围绕正前方旋转-20度，投掷武器的方向
	FVector ImpulseDirection = MeshRight = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandomRotation  = FMath::FRandRange(0.f, 30.f);
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 20'000.f;// 冲击方向以及力度

	// 施加冲击力
	GetItemMesh()->AddImpulse(ImpulseDirection);

	// 下落中
	bFalling = true;

	// 设置定时器
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);

	EnableGlowMaterial();
}


void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
	StartPulseTimer();
}

void AWeapon::DecrementAmmo()
{
	if (Ammo - 1 <= 0)
	{
		Ammo = 0;
	}
	else
	{
		--Ammo;
	}
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagazineCapacity, TEXT("填充的弹药数量超过了武器自身弹匣容量"));
	Ammo += Amount;
}

bool AWeapon::ClipIsFull()
{
	return Ammo >= MagazineCapacity;
}
