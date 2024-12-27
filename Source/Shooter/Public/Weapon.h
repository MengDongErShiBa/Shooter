// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSection;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairMiddle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairBottom;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairTop;

	// 射速
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToHide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoMatic;
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

	// 重写初始化函数
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	// 完成滑膛
	void FinishMovingSlide();

	// 更新枪膛滑动
	void UpdateSlideDisplacement();
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

	// 武器属性表格
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	int32 PreviousMaterialIndex;

	// 武器准星 Texture
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairMiddle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairBottom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairTop;

	// 武器射速
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	float AutoFireRate;

	// 枪口的粒子效果
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	// 开火声音
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	USoundCue* FireSound;

	// 要隐藏在武器网格上的骨头的名称
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	FName BoneToHide;

	// 手枪射击时滑梯向后推的量
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta=(AllowPrivateAccess = "true"))
	float SlideDisplacement;

	// Curve for the slide displacement  手机上膛滑动曲线
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta=(AllowPrivateAccess = "true"))
	UCurveFloat* SlideDisplacementCurve;

	// Timer handle for updating SlideDisplacement 用于更新滑动位移的定时器
	FTimerHandle SlideTimer;

	// Time for displaceing the slide during pistol fire,手枪射击过程中滑动枪膛得时间
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta=(AllowPrivateAccess = "true"))
	float SlideDisplacementTime;

	// 手枪滑膛时为true
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta=(AllowPrivateAccess = "true"))
	bool bMovingSlide;

	// 枪膛滑动得最大距离
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta=(AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;

	// 最大后坐力旋转
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta=(AllowPrivateAccess = "true"))
	float MaxRecoilRotation;
	
	// 手枪射击时后坐力旋转的量
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta=(AllowPrivateAccess = "true"))
	float RecoilRotation;

	// 自动射击为True
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	bool bAutoMatic;
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
	FORCEINLINE void SetReloadMontageSection(FName Section) { ReloadMontageSection = Section; }
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE void SetClipBoneName(FName BoneName) { ClipBoneName = BoneName; }
	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }
	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }
	FORCEINLINE bool GetAutoMatic() const { return bAutoMatic; }

	void StartSlideTimer();

	// 重载弹药
	void ReloadAmmo(int32 Amount);

	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }

	bool ClipIsFull();
};
