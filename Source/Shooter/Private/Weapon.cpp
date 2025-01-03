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
	ClipBoneName(TEXT("smg_clip")),
	SlideDisplacement(0),
	SlideDisplacementTime(0.2f),
	bMovingSlide(false),
	MaxSlideDisplacement(4.f),
	MaxRecoilRotation(20.f),
	bAutoMatic(true)
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

	// 刷新枪膛位置
	UpdateSlideDisplacement();
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

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FString WeaponTablePath { TEXT("/Script/Engine.DataTable'/Game/_Game/DataTable/WeaponDataTable.WeaponDataTable'") };

	if (UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath)))
	{
		FWeaponDataTable* WeaponDataRow = nullptr;
		
		switch (WeaponType)
		{
		case EWeaponType::EWT_SubmachineGun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"), TEXT(""));
			break;
		case EWeaponType::EWT_AssaultRifle:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
			break;
		case EWeaponType::EWT_Pistol:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
			break;
		default:
			break;
		}

		if (WeaponDataRow)
		{
			// 设置武器属性
			AmmoType = WeaponDataRow->AmmoType;
			Ammo = WeaponDataRow->WeaponAmmo;
			MagazineCapacity = WeaponDataRow->MagazineCapacity;
			SetPickupSound(WeaponDataRow->PickupSound);
			SetEquipSound(WeaponDataRow->EquipSound);
			GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
			SetItemName(WeaponDataRow->ItemName);
			SetIconItem(WeaponDataRow->InventoryIcon);
			SetAmmoIcon(WeaponDataRow->AmmoIcon);
			
			SetMaterialInstance(WeaponDataRow->MaterialInstance);
			PreviousMaterialIndex = GetMaterialIndex();
			GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
			SetMaterialIndex(WeaponDataRow->MaterialIndex);
			SetClipBoneName(WeaponDataRow->ClipBoneName);
			SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);
			GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);

			CrosshairMiddle = WeaponDataRow->CrosshairMiddle;
			CrosshairLeft = WeaponDataRow->CrosshairLeft;
			CrosshairRight = WeaponDataRow->CrosshairRight;
			CrosshairTop = WeaponDataRow->CrosshairTop;
			CrosshairBottom = WeaponDataRow->CrosshairBottom;
			AutoFireRate = WeaponDataRow->AutoFireRate;
			MuzzleFlash = WeaponDataRow->MuzzleFlash;
			FireSound = WeaponDataRow->FireSound;
			BoneToHide = WeaponDataRow->BoneToHide;
			bAutoMatic = WeaponDataRow->bAutoMatic;
			Damage = WeaponDataRow->Damage;
			HeadShotDamage = WeaponDataRow->HeadShotDamage;
		}
		
		if (GetMaterialInstance())
		{
			// 创建动态材质实例
			SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
			GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
			GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());
			EnableGlowMaterial();
		}
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (BoneToHide != FName(""))
	{
		GetItemMesh()->HideBoneByName(BoneToHide, PBO_None);
	}
	
}

void AWeapon::FinishMovingSlide()
{
	bMovingSlide = false;
}

void AWeapon::UpdateSlideDisplacement()
{
	if (SlideDisplacementCurve && bMovingSlide)
	{
		const float ElapsedTime { GetWorldTimerManager().GetTimerElapsed(SlideTimer) };
		const float CurveValue { SlideDisplacementCurve->GetFloatValue(ElapsedTime) };

		SlideDisplacement = CurveValue * MaxSlideDisplacement;
		RecoilRotation = CurveValue * MaxRecoilRotation;
	}
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

void AWeapon::StartSlideTimer()
{
	bMovingSlide = true;
	GetWorldTimerManager().SetTimer(SlideTimer, this, &AWeapon::FinishMovingSlide, SlideDisplacementTime);
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
