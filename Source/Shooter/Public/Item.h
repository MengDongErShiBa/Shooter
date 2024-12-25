// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

/**
 * 物品级别
 */
UENUM()
enum class  EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "残破"),
	EIR_Common UMETA(DisplayName = "平凡"),
	EIR_UnCommon UMETA(DisplayName = "非凡"),
	EIR_Rare UMETA(DisplayName = "稀有"),
	EIR_Legendary UMETA(DisplayName = "传奇"),

	EIR_MAX UMETA(DisplayName = "默认最大值")
};

/**
 * 物品状态
 */
UENUM()
enum class  EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "可拾取"),
	EIS_EquipInterping UMETA(DisplayName = "装备插值中"),
	EIS_PickedUp UMETA(DisplayName = "拾取完毕"),
	EIS_Equipped UMETA(DisplayName = "装备完毕"),
	EIS_Falling UMETA(DisplayName = "掉落"),
	
	EIR_MAX UMETA(DisplayName = "默认最大值")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "子弹"),
	EIT_Weapon UMETA(DisplayName = "武器"),

	EIT_Max UMETA(DisplayName = "最大值")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	// 发光颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStarts;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture* IconBackground;
};


UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 球体区域重叠调用
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,  AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 球体区域结束重叠调用
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 设置基于稀有度的星星数量
	void SetActiveStarts();

	// 根据状态设置Item的属性
	virtual void SetItemProperties(EItemState State);

	// Called when ItemInterpTimer is finished
	// 插值定时器完成调用
	void FinishInterping();

	// 处理物品移动插值
	void ItemInterp(float DeltaTime);

	// 获取插值位置
	FVector GetInterpLocation();

	// 播放声音
	void PlayPickupSound(bool bForcePlaySound = false);
	
	// 初始化自定义深度
	virtual void InitializeCustomDepth();

	// C++版本的构造，早于BeginPlay
	virtual void OnConstruction(const FTransform& Transform) override;

	// 开启发光材质
	void EnableGlowMaterial();

	void UpdatePulse();

	// 重置发光频率
	void RestPulseTimer();

	void StartPulseTimer();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 在角色调用
	void PlayEquipSound(bool bForcePlaySound = false);

private:
	// Item的骨骼Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;
	
	// 用于碰撞检测，显示HUD
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	// 玩家看到道具时的弹出窗口
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	// 重叠时启用项目追踪
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	// 应用于UI显示Item名称
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	FString ItemName;

	// 数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	int32 ItemCount;

	// 物品稀有度，用于显示UI星
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	TArray<bool> ActiveStarts;

	// Item状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	EItemState ItemState;

	// 插值时用于ItemZ轴的曲线
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve;

	// 插值起始的本地位置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;

	// 摄像机目标位置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	// 是否在插值
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	bool bInterping;

	// 开始插值时调用
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	FTimerHandle ItemInterpTimer;

	// Z曲线持续时长
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	float ZCurveTime;

	// 角色指针
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta=(AllowPrivateAccess = "true"))
	class AShooterCharacter* Character;

	// X and Y for the Item while interping in the EquipInterping state
	// Item插值过程中XY的值
	float ItemInterpX;
	float ItemInterpY;

	// 相机和Item之间初始Yaw偏移
	float InterpInitialYawOffset;

	// Item 插值缩放曲线
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;

	// 播放声音在拾取物品的时候
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	class USoundCue* PickupSound;

	// 装备时播放声音
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	USoundCue* EquipSound;

	// 物品类型枚举
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	EItemType ItemType;

	// 插入位置的索引
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	int32 InterpLocIndex;

	// 在运行时可更改的材料索引
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	// 动态创建  运行时可更改的材质实例
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	// 蓝图中选择  和动态实例一起使用
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	bool bCanChangeCustomDepth;

	// 驱动动态材质参数变化
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	class UCurveVector* PulseCurve;

	// 插值时  材质闪烁曲线，拾取时闪一下
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	class UCurveVector* InterpPulseCurve;

	// 频率
	FTimerHandle PulseTimer;

	// 时长
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	float PulseCurveTime;

	// 发光强度
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	float GlowAmount;

	// 菲尼尔强度
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	float FresnelExponent;

	// 菲尼尔反射强度
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess = "true"))
	float FresnelReflectFraction;

	// 库存系统中，此项目的背景
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta=(AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;

	// 库存系统中，Item的Icon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta=(AllowPrivateAccess = "true"))
	UTexture2D* IconItem;
	
	// 库存系统中，子弹Icon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta=(AllowPrivateAccess = "true"))
	UTexture2D* IconAmmoItem;

	// 在库存中的槽位
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta=(AllowPrivateAccess = "true"))
	int32 SlotIndex;

	/** 当角色的库存是满的时候 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta=(AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;

	// Item稀有度表
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory, meta=(AllowPrivateAccess = "true"))
	class UDataTable* ItemRarityDataTable;

public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	void SetItemState(EItemState State);
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	FORCEINLINE void SetCharacter(AShooterCharacter* Char) { Character = Char; }
	FORCEINLINE void SetCharacterInventoryFull(bool bFull) { bCharacterInventoryFull = bFull; }

	// called from the AShooterCharacter class
	// 从ShooterCharacter调用，开始曲线移动
	void StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound = false);

	// 开启自定义深度
	virtual void EnableCustomDepth();

	// 关闭自定义深度
	virtual void DisableCustomDepth();
	
	// 关闭发光材质
	void DisableGlowMaterial();
};

