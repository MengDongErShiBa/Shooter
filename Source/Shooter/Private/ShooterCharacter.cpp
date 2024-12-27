// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "Ammo.h"
#include "Item.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	// 基础转弯/上下速率
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	// 瞄准和非瞄准的旋转速率
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// 鼠标灵敏度
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingTurnRate(0.6f),
	MouseAimingLookUpRate(0.6f),
	bAiming(false),	
	// 相机缩放
	CameraDefaultFOV(0.F),
	CameraZoomedFOV(25.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),
	// 瞄准扩散因数字
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	// 子弹设计变量
	ShootTimerDuration(0.05f),
	bFiringBullet(false),
	// 自动射击速率，这里还要注意他和射击扩散的时间大概为1:0.5
	// 自动射击速率为 1， 扩散时间为0.5
	bFireButtonPressed(false),
	bShouldFire(true),
	// 是否每帧检查Item
	bShouldTraceForItems(false),
	// 相机插值位置参数
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
	// 子弹数量
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	// 战斗状态
	CombatState(ECombatState::ECS_Unoccupied),
	bCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(300.f),
	StandingCapsuleHalfHeight(88.f),
	CrouchingCapsuleHalfHeight(44.f),
	BaseGroundFriction(2.f),
	CrouchingGroundFriction(100.f),
	// 是否按下瞄准按键
	bAimingButtonPressed(false),
	// 拾取音效timer属性
	bShouldPlayPickSound(true),
	bShouldPlayEquipSound(true),
	PickupSoundResetTime(0.2f),
	EquipSoundResetTime(0.2f),
	// Icon 动画属性
	HighlightedSlot(-1)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 创建弹簧臂，如果发生碰撞将拉进和角色的距离 pulls in towards the character if there is a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// 附加到Root
	CameraBoom->SetupAttachment(RootComponent);
	// 设置长度为300
	CameraBoom->TargetArmLength = 180.f;
	// 使用玩家Controller的旋转
	CameraBoom->bUsePawnControlRotation = true;
	// 弹簧臂偏移 玩家偏左
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// 摄像机不适用玩家旋转，只跟随弹簧臂的旋转
	FollowCamera->bUsePawnControlRotation = false;

	// 不适用控制器旋转，控制器旋转只影响摄像机
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;// 使用控制器旋转
	bUseControllerRotationRoll = false;

	// 角色是否应该根据移动方向自动调整其朝向
	GetCharacterMovement()->bOrientRotationToMovement = false;
	// 角色旋转速率
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	// 角色跳跃高度
	GetCharacterMovement()->JumpZVelocity = 600.f;
	// 角色在空中的控制方向的能力，这里是百分之二十
	GetCharacterMovement()->AirControl = 0.2f;

	// 创建手部场景组件
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	// 创建武器插值场景组件
	WeaponInterComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
	WeaponInterComp->SetupAttachment(GetFollowCamera());

	// 其他物品插值组件
	InterpCom1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
	InterpCom1->SetupAttachment(GetFollowCamera());

	InterpCom2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
	InterpCom2->SetupAttachment(GetFollowCamera());

	InterpCom3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
	InterpCom3->SetupAttachment(GetFollowCamera());

	InterpCom4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
	InterpCom4->SetupAttachment(GetFollowCamera());

	InterpCom5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
	InterpCom5->SetupAttachment(GetFollowCamera());

	InterpCom6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
	InterpCom6->SetupAttachment(GetFollowCamera());

}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		// 获取摄像机有效距离
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	// 创建默认武器并装备
	EquipWeapon(SpawnDefaultWeapon());
	// 添加到库存
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	// 关闭深度和发光效果
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);

	// 初始化弹药
	InitializeAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	// 为每个插值位置创建位置，添加到数组
	InitializeInterpLocations();
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// 寻找前进方向
		const FRotator Rotation { Controller->GetControlRotation() };
		const FRotator YawRotation { 0, Rotation.Yaw, 0 };

		const FVector Direction { FRotationMatrix { YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// 寻找向右方向
		const FRotator Rotation { Controller->GetControlRotation() };
		const FRotator YawRotation { 0, Rotation.Yaw, 0 };

		const FVector Direction { FRotationMatrix { YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// 根据默认的速率计算此帧需要旋转的大小
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor {};
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor {};
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	// 根据默认速录计算此帧需要俯仰的度数 度数/秒数 * 秒/帧数
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr) return;

	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		// 播放声音
		PlayFireSound();
		// 发射子弹
		SendBullet();
		// 播放开火蒙太奇
		PlayGunFireMontage();
		// 减少子弹
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();

		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			// 开始滑膛效果
			EquippedWeapon->StartSlideTimer();
		}
		
		// 准心扩散
		StartCrosshairBulletFire();
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// 检查准心是否命中
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		// 暂定结束位置为命中位置，具体位置还需从枪口进行追踪
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else
	{
		// OutBeamLocation是射线追踪的结束位置
	}

	// 执行第二次追踪，这次从枪口开始
	// 武器追踪碰撞
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd { OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };
	// 检查武器通道是否有碰撞
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}

	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	// 装填状态无法瞄准
	if (CombatState != ECombatState::ECS_Reloading && CombatState != ECombatState::ECS_Equipping)
	{
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	// 是否按下瞄准键
	if (bAiming)
	{
		// 设置缩小视角
		// 通过插值设置当前距离
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		// 设置放大视角
		// 通过插值设置当前距离
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;

	// 计算基于速度的准心扩散因数
	// GetMappedRangeValueClamped用于将一个值从一个范围线性映射到另一个范围，并确保结果值在目标范围内。
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	// 计算空中float DeltaTime的准心扩散因数
	if (GetCharacterMovement()->IsFalling())
	{
		// 在空中缓慢展开准心
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		// 回到地面马上收缩
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);   
	}

	// 计算瞄准因数
	if (bAiming)
	{
		// 瞄准时迅速收缩
		// Shrink crosshair a small amount very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	}
	else
	{
		// 恢复正常
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	// 触发射击0.05秒内为真                                                                                            
	if (bFiringBullet)
	{
		// 射击中迅速扩散
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		// 设计后迅速收缩
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}
	
	// 准心扩散系数 + 准心
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor
		+ CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	// 设置一个计时器。这段代码的功能是在指定的时间间隔后执行一个特定的函数。
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimerDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;
	
	if (EquippedWeapon == nullptr) return;
	
	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed && EquippedWeapon->GetAutoMatic())
		{
			FireWeapon();
		}
	}
	else
	{
		// 重载武器
		ReloadWeapon();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// 获取当前游戏视口大小
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// 获取准心在屏幕上的位置
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// 获取准星的世界坐标和方向
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		// 从准心位置发出射线
		// 开始位置
		const FVector Start{ CrosshairWorldPosition};
		// 结束位置
		const FVector End { Start + CrosshairWorldDirection * 50'000.f };
		// 命中位置
		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(
			OutHitResult,
			Start,
			End,
			ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
		
	}

	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		// 射线检测是否瞄准Item
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);

		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem))
			{
				if (HighlightedSlot == -1)
				{
					// 当前没有高亮插槽，高亮显示一个
					HighlightInventorySlot();
				}
			}
			else
			{
				if (HighlightedSlot != -1)
				{
					// 取消高亮
					UnHighlightInventorySlot();
				}
			}

			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}

			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				// 显示Item的拾取UI
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				if (Inventory.Num() >= INVENTORY_CAPACITY)
				{
					// 库存已满
					TraceHitItem->SetCharacterInventoryFull(true);
				}
				else
				{
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			}

			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					// 和上一帧的Item不一样或是NULL
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}

			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// 没有重叠Item，不显示任何UI
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		// 生成Actor
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
{
	if (WeaponToEquip)
	{
		// 设置武器碰撞通道为忽略所有
		
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));

		if (HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		if (EquippedWeapon == nullptr)
		{
			// -1 == 现在没有装备武器 不需要改变Icon动画
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if (!bSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}

		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		// 组件将保持其在世界空间中的位置和旋转
		// 保持缩放
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		// 将组件从附加组件中分离出来
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();

		// EquippedWeapon = nullptr;
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	// 只有空闲中可以拾取
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	if (TraceHitItem)
	{
		// auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		// SwapWeapon(TraceHitWeapon);
		TraceHitItem->StartItemCurve(this, true);

		if (TraceHitItem->GetPickupSound())
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickupSound());
		}
		
		// 置为空，防止再次触发曲线
		TraceHitItem = nullptr;
	}
}

void AShooterCharacter::SelectButtonReleased()
{
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	
	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	// 装备后将当前命中Item设置为空
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	return EquippedWeapon->GetAmmo() > 0;
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}
	return false;
}

void AShooterCharacter::PlayFireSound()
{
	if (EquippedWeapon == nullptr) return;
	
	if (EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void AShooterCharacter::SendBullet()
{
	if (EquippedWeapon == nullptr) return;
	
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if (EquippedWeapon->GetMuzzleFlash())
		{
			// 播放枪口粒子效果
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}

			//烟雾效果
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		// 播放开火蒙太奇
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon == nullptr) return;

	// 检查是否有对应型号的弹药
	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull())
	{
		// 装填中取消瞄准
		if (bAiming)
		{
			StopAiming();
		}
		
		if (ReloadMontage)
		{
			CombatState = ECombatState::ECS_Reloading;
			
			if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Play(ReloadMontage);
				AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
			}
		}
	}
}

void AShooterCharacter::FinishReloading()
{
	// 更新战斗状态
	CombatState = ECombatState::ECS_Unoccupied;

	// 装填完毕后瞄准建还在按下
	if (bAimingButtonPressed)
	{
		Aim();
	}

	if (EquippedWeapon == nullptr) return;

	const auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		// 角色已经装备的弹药数量
		int32 CarriedAmmo = AmmoMap[AmmoType];
		// 弹匣中弹药空闲数量
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// 将所有弹药加入弹匣
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

void AShooterCharacter::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimingButtonPressed)
	{
		Aim();
	}
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	// 装备武器的骨骼下标
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	// 存储弹匣位置
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	// 附加规则  保持相对
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}

	GetCharacterMovement()->MaxWalkSpeed = bCrouching ? CrouchMovementSpeed : BaseMovementSpeed;
	GetCharacterMovement()->GroundFriction = bCrouching ? CrouchingGroundFriction : BaseGroundFriction;
}

void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight = bCrouching ? CrouchingCapsuleHalfHeight : StandingCapsuleHalfHeight;

	// 插值得到胶囊体半高
	const float InterpHalfHeight { FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f) };

	// 蹲伏时为负值，站立时为正值
	const float DeltaCapsuleHalfHeight { InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };
	// 网格偏移
	const FVector MeshOffset { 0.f, 0.f, -DeltaCapsuleHalfHeight };
	
	GetMesh()->AddLocalOffset(MeshOffset);
	
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	//GetFollowCamera()->SetFieldOfView(CameraZoomedFOV);
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	//GetFollowCamera()->SetFieldOfView(CameraDefaultFOV);
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	// 检查是否包含子弹类型
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		AmmoMap[Ammo->GetAmmoType()] += Ammo->GetItemCount();
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation { WeaponInterComp, 0};
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterLoc1 { InterpCom1, 0};
	InterpLocations.Add(InterLoc1);

	FInterpLocation InterLoc2 { InterpCom2, 0};
	InterpLocations.Add(InterLoc2);

	FInterpLocation InterLoc3 { InterpCom3, 0};
	InterpLocations.Add(InterLoc3);

	FInterpLocation InterLoc4 { InterpCom4, 0};
	InterpLocations.Add(InterLoc4);

	FInterpLocation InterLoc5 { InterpCom5, 0};
	InterpLocations.Add(InterLoc5);

	FInterpLocation InterLoc6 { InterpCom6, 0};
	InterpLocations.Add(InterLoc6);
}

void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	// if ((CurrentItemIndex == NewItemIndex) || (NewItemIndex >= Inventory.Num()) || CombatState != ECombatState::ECS_Unoccupied) return;

	const bool bCanExchangeItems =
		CurrentItemIndex != NewItemIndex &&
		NewItemIndex < Inventory.Num() &&
		(CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping);
	
	if (bCanExchangeItems)
	{
		if (bAiming)
		{
			StopAiming();
		}
		
		auto OldEquippedWeapon = EquippedWeapon;
		auto NewEquippedWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);

		EquipWeapon(NewEquippedWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		NewEquippedWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && EquipMontage)
		{
			AnimInstance->Montage_Play(EquipMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}
		NewEquippedWeapon->PlayEquipSound(true);
	}
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}

	// 返回下一个位置
	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}

	// 库存中没有空位
	return -1;
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot { GetEmptyInventorySlot() };
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}
	
	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;

	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
	
}

void AShooterCharacter::StartPickSoundTimer()
{
	bShouldPlayPickSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 处理瞄准时插值变焦
	CameraInterpZoom(DeltaTime);

	// 根据瞄准改变旋转等灵敏度
	SetLookRates();

	// 计算准心扩散系数
	CalculateCrosshairSpread(DeltaTime);

	// 检查重叠Item数量
	TraceForItems();
	// 插值设置胶囊体半高
	InterpCapsuleHalfHeight(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	// 基础输入
	PlayerInputComponent->BindAxis("MoveForward" , this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	// PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	// PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	// 跳跃
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// 开火
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	// 瞄准
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	// 选择
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	// 重载弹药
	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	// 蹲伏按钮按下
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

// FVector AShooterCharacter::GetCameraInterpLocation()
// {
// 	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
// 	const FVector CameraForward{ FollowCamera->GetForwardVector() };
// 	// Desired = CameraWorldLocation + Froward * A + Up * B
// 	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
// }

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();
	
	if (auto Weapon = Cast<AWeapon>(Item))
	{
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else
		{
			// 库存已满  交换武器
			SwapWeapon(Weapon);
		}
	}

	if (auto Ammo = Cast<AAmmo>(Item))
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}

