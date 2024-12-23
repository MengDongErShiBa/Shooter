// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"

UENUM(Blueprintable)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "冲锋枪"),
	EWT_AssaultRifle UMETA(DisplayName = "突击步枪"),

	EWT_Max UMETA(DisplayName = "最大值")
};

/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();

	virtual void Tick(float DeltaSeconds) override;
protected:
	// 停止下落
	void StopFalling();
private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	// 当前武器的子弹数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	int32 Ammo;

	// 当前武器的最大弹药容量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	// 武器类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	// 武器弹药类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	// 重载弹药蒙太奇名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	FName ReloadMontageSection;
	
	// 在重新加载弹药时为true
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	bool bMovingClip;

	// 可移动的骨骼名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	FName ClipBoneName;
public:
	// 扔出武器，施加冲击力
	void ThrowWeapon();

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

	// 在Character开火时调用 Called from character class when firing weapon
	void DecrementAmmo();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }

	void ReloadAmmo(int32 Amount);

	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }

	bool ClipIsFull();
};
