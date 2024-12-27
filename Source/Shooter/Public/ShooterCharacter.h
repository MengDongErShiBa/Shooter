// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "无事发生"),
	ECS_FireTimerInProgress UMETA(DisplayName = "正在开火"),
	ECS_Reloading UMETA(DisplayName = "重新装载"),
	ECS_Equipping UMETA(DisplayName = "装备中"),

	ECS_MAX UMETA(DisplayName = "默认最大值")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	// 场景组件，用于指定位置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	// 插入的数量
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};

class AAmmo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHightlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * Call for forwards/backwards input
	 * @param Value 
	 */
	void MoveForward(float Value);

	/**
	 * Call for rightwards/leftwards input
	 * @param Value 
	 */
	void MoveRight(float Value);

	/**
	 * 通过输入调用给定的速率旋转
	 * @param Rate 标准化的速率，即1.0意味着百分之的旋转
	 */
	void TurnAtRate(float Rate);

	/**
	 * 根据鼠标X的移动来旋转控制器
	 * @param Rate 鼠标移动的输入值 标准化的速率，即1.0意味着百分之的旋转
	 */
	void Turn(float Value);

	/**
	 * 根据鼠标Y的移动来旋转控制器
	 * @param Rate 鼠标移动的输入值 标准化的速率，即1.0意味着百分之的旋转
	 */
	void LookUp(float Value);

	/**
	 * 通过输入调用给定的上下速率
	 * @param Rate 标准化的速率，和Turn一样
	 */
	void LookUpAtRate(float Rate);

	/**
	 * 开火按钮按下
	 */
	void FireWeapon();

	/**
	 * 获取命中爆炸位置
	 * @param MuzzleSocketLocation 枪口位置
	 * @param OutBeamLocation 输出爆炸位置
	 * @return 
	 */
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	/**
	 * 瞄准按钮按下
	 */
	void AimingButtonPressed();

	/**
	 * 瞄准按钮松开
	 */
	void AimingButtonReleased();

	// 摄像机插值缩小视角
	void CameraInterpZoom(float DeltaTime);

	// 基于瞄准设置基础旋转率和基础上下速率
	void SetLookRates();

	// 计算准心扩散
	void CalculateCrosshairSpread(float DeltaTime);

	// 开始准星射击
	void StartCrosshairBulletFire();

	// 完成射击
	UFUNCTION()
	void FinishCrosshairBulletFire();

	// 射击按钮按下
	void FireButtonPressed();

	// 射击按钮松开
	void FireButtonReleased();

	//开始射击定时器
	void StartFireTimer();

	// 自动射击
	UFUNCTION()
	void AutoFireReset();

	// 对准心下的物体进行追踪
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	// 根据重叠Item数量进行射线检查
	void TraceForItems();

	// 创建默认武器
	class AWeapon* SpawnDefaultWeapon();

	// 装备武器
	void EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping = false);

	 // Detach weapon and let it fall to the ground
	// 分离武器并让他掉落地面
	void DropWeapon();

	// 选择E按下
	void SelectButtonPressed();
	// 选择E抬起
	void SelectButtonReleased();

	// Drops currently equipeed weapon and equips trace hit item
	// 掉落当前装备的武器&装备当前追踪命中的武器
	void SwapWeapon(AWeapon* WeaponToSwap);

	// 初始化弹药Map
	void InitializeAmmoMap();

	// 武器是否有弹药
	bool WeaponHasAmmo();

	// Checks to see if we have ammo of the EquippedWeapon's ammo type
	// 检查是否携带武器类型的弹药
	bool CarryingAmmo();
	
#pragma region 开火
	// 播放开火音效
	void PlayFireSound();

	// 发射子弹
	void SendBullet();

	// 播放开火蒙太奇
	void PlayGunFireMontage();
#pragma endregion 

	// 重载弹药按钮按下 R
	void ReloadButtonPressed();

	// 重载弹药
	void ReloadWeapon();

	// 动画蓝图通过通知调用 完成重载弹药动作
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	// 动画蓝图通过通知调用 完成装备
	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	// 动画蓝图内通过通知调用 抓住弹匣
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	// 动画蓝图通知调用 释放弹匣
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	// 蹲伏按钮按下
	void CrouchButtonPressed();

	virtual void Jump() override;

	// 蹲伏或站立时插值胶囊体半高
	void InterpCapsuleHalfHeight(float DeltaTime);

	// 开始瞄准
	void Aim();
	
	// 停止瞄准
	void StopAiming();

	// 拾取子弹
	void PickupAmmo(AAmmo* Ammo);

	// 初始化插值位置列表
	void InitializeInterpLocations();

	// Begin 快捷键按下
	void FKeyPressed();
	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();
	// End 快捷键按下

	// 交换库存物品
	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);

	// 获取空的插槽
	int32 GetEmptyInventorySlot();

	// 高亮显示插槽
	void HighlightInventorySlot();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/**
	 * Camera boom positioning the camera behind the character
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/**
	 * 摄像机
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "ture"))
	UCameraComponent* FollowCamera;

	/**
	 * 基础旋转速率 度/秒 比例会影响最终的转化率 转弯速率
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/**
	 * 基础上下速率 度/秒 比例会影响最终的转化率 上下视角速率
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	// 不瞄准时的转弯速率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float HipTurnRate;

	// 非瞄准下上下速率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float HipLookUpRate;

	// 瞄准下转弯速率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float AimingTurnRate;

	// 瞄准下上下速率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	// 鼠标在非瞄准下的旋转速率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;
	
	// 鼠标在非瞄准下的上下速率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	// 鼠标在瞄准下的旋转速率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	// 鼠标在瞄准下的上下速率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;
	
	/**
	 * 开火蒙太奇
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* HipFireMontage;

	/**
	 * 命中粒子
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	/**
	 * 子弹轨迹效果
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	/**
	 * 是否瞄准
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta=(AllowPrivateAccess = "true"))
	bool bAiming;

	/**
	 * 默认相机视野
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float CameraDefaultFOV;

	/**
	 * 相机放大距离
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float CameraZoomedFOV;

	// 当前帧相机距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float CameraCurrentFOV;

	// 瞄准缩放视角插值速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	// 准心扩散系数
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	// 准心速度因数
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	// 空中准心因数
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	// 瞄准时因数
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	// 设计时因子
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	// 开火时长
	float ShootTimerDuration;

	// 是否在射击
	bool bFiringBullet;

	// 准星散开射击定时器
	FTimerHandle CrosshairShootTimer;

	// 鼠标左键开火按钮是否按下
	bool bFireButtonPressed;

	// 可以开火为True，等待计时器为false
	bool bShouldFire;

	// 射击间距计时器
	FTimerHandle AutoFireTimer;

	// 是否可以每帧检查Items
	bool bShouldTraceForItems;

	// 重叠Item数量
	int8 OverlappedItemCount;

	// 最后一帧检测命中的Item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta=(AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;

	// 当前装备的武器
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta=(AllowPrivateAccess = "true"))
	class AWeapon* EquippedWeapon;

	// 在蓝图中设置的默认武器
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Combat, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	// The item currently hit by our trace in TraceForItems (could be null)
	// 当前追踪命中的Item，可以为空
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= Combat, meta=(AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	// 相机到插值位置的距离
	// Distance outward form the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	// 相机到插值位置向上的距离
	// Distance upward from the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	// 存放不同类型的子弹
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Item, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	// 初始9mm子弹数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	// 初始步枪子弹数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	// 战斗状态，只有在无事发生状态才可以装填或开火
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	// 重新装载弹药蒙太奇
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	// 重新装载弹药蒙太奇
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* EquipMontage;

	// 武器骨骼位移，重新加载时第一次抓住弹匣，弹匣的位置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta=(AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	// 重新加载时附加到角色手部
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta=(AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	// 是否蹲下
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess))
	bool bCrouching;

	// 基础移动速度
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess))
	float BaseMovementSpeed;

	// 蹲伏移动速度
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess))
	float CrouchMovementSpeed;

	// 胶囊的当前半高
	float CurrentCapsuleHalfHeight;

	// 非蹲伏时为胶囊体一半高
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement, meta=(AllowPrivateAccess))
	float StandingCapsuleHalfHeight;

	// 蹲伏时为胶囊体一半高
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement, meta=(AllowPrivateAccess))
	float CrouchingCapsuleHalfHeight;

	// 基础地面摩擦力  非蹲伏时的地面摩擦力
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement, meta=(AllowPrivateAccess))
	float BaseGroundFriction;

	// 蹲伏地面摩擦力 蹲伏时地面摩擦力
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement, meta=(AllowPrivateAccess))
	float CrouchingGroundFriction;

	// 瞄准按钮是否按下
	bool bAimingButtonPressed;

	// 武器插值
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	USceneComponent* WeaponInterComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	USceneComponent* InterpCom1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	USceneComponent* InterpCom2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	USceneComponent* InterpCom3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	USceneComponent* InterpCom4;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	USceneComponent* InterpCom5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	USceneComponent* InterpCom6;

	// 插值位置结构数组
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<FInterpLocation> InterpLocations;

	// begin 限制随意播放声音
	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickSound;
	bool bShouldPlayEquipSound;

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

	// 播放音效时长
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Items, meta=(AllowPrivateAccess="true"))
	float PickupSoundResetTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Items, meta=(AllowPrivateAccess="true"))
	float EquipSoundResetTime;
	// end 限制随意播放声音

	// 用于库存的Item数组
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= Inventory, meta=(AllowPrivateAccess="true"))
	TArray<AItem*> Inventory;

	// 库存大小
	const int32 INVENTORY_CAPACITY { 6 };

	// 在装备时发送槽位信息给InventoryBar
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;

	// 用于发送播放图标动画的插槽信息的委托
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FHightlightIconDelegate HighlightIconDelegate;

	// 当前高亮的插槽
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= Inventory, meta=(AllowPrivateAccess="true"))
	int32 HighlightedSlot;
public:
	/**
	 * 获取弹簧臂
	 * @return 弹簧臂
	 */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/**
	 * 获取摄像机
	 */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// 是否在瞄准
	FORCEINLINE bool GetAiming() const { return bAiming; }

	// 获取准心扩散系数
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	// 增加或减少重叠Item数量
	void IncrementOverlappedItemCount(int8 Amount);

	// 获取相机函数插值位置  不再需要
	// FVector GetCameraInterpLocation();

	// 拾取Item   调用在Item
	void GetPickupItem(AItem* Item);

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }

	FORCEINLINE bool GetCrouching() const { return bCrouching; }

	FInterpLocation GetInterpLocation(int32 Index);

	// 返回数组中ItemCount最小的位置
	int32 GetInterpLocationIndex();

	// 增加指定位置的ItemCount
	void IncrementInterpLocItemCount(int32 Index, int32 Amount);

	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickSound; }
	FORCEINLINE bool ShouldEquipSound() const { return bShouldPlayEquipSound; }

	// Begin Item中调用
	void StartPickSoundTimer();
	void StartEquipSoundTimer();
	// End Item中调用

	// 取消高亮显示
	void UnHighlightInventorySlot();

	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
};