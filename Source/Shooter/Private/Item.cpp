// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AItem::AItem():
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup),
	// Z轴插值
	ZCurveTime(0.7f),
	ItemInterpStartLocation(FVector(0.f)),
	CameraTargetLocation(FVector(0.f)),
	bInterping(false),
	ItemInterpX(0.f),
	ItemInterpY(0.f),
	InterpInitialYawOffset(0.f),
	ItemType(EItemType::EIT_Max),
	InterpLocIndex(0),
	MaterialIndex(0),
	bCanChangeCustomDepth(true),
	// Dynamic Material Parameters
	GlowAmount(150.f),
	FresnelExponent(3.f),
	FresnelReflectFraction(4.f),
	PulseCurveTime(5.f),
	SlotIndex(0),
	bCharacterInventoryFull(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	// 设置Mesh为Root
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	// 忽略所有碰撞通道
	CollisionBox->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		// 隐藏拾取HUD
		PickupWidget->SetVisibility(false);
	}

	// 设置基础稀有度
	SetActiveStarts();

	// 设置重叠事件
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	// 设置Item属性基于基础状态
	SetItemProperties(ItemState);

	// 关闭自定义深度
	InitializeCustomDepth();

	StartPulseTimer();
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(1);
			ShooterCharacter->UnHighlightInventorySlot();
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(-1);
		}
	}
}

void AItem::SetActiveStarts()
{
	// 不是用0 ，不太清楚需要看看，就是简单的不用0...
	for(int32 i = 0; i <= 5; i++)
	{
		ActiveStarts.Add(false);
	}

	switch (ItemRarity)
	{
	case EItemRarity::EIR_Damaged:
		ActiveStarts[1] = true;
		break;
	case EItemRarity::EIR_Common:
		ActiveStarts[1] = true;
		ActiveStarts[2] = true;
		break;
	case EItemRarity::EIR_UnCommon:
		ActiveStarts[1] = true;
		ActiveStarts[2] = true;
		ActiveStarts[3] = true;
		break;
	case EItemRarity::EIR_Rare:
		ActiveStarts[1] = true;
		ActiveStarts[2] = true;
		ActiveStarts[3] = true;
		ActiveStarts[4] = true;
		break;
	case EItemRarity::EIR_Legendary:
		ActiveStarts[1] = true;
		ActiveStarts[2] = true;
		ActiveStarts[3] = true;
		ActiveStarts[4] = true;
		ActiveStarts[5] = true;
		break;
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch(State)
	{
	case EItemState::EIS_Pickup: //可拾取状态
		// 设置ItemMesh属性
		ItemMesh->SetSimulatePhysics(false);// 关闭模拟物理
		ItemMesh->SetEnableGravity(false); // 关闭重力
		ItemMesh->SetVisibility(true);	// 可见
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);// 忽略所有通道碰撞
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞
		// 设置球体碰撞
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Overlap); // 响应方式为重叠
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // gpt 问下
		// 设置碰撞Box属性
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EItemState::EIS_Equipped:
		PickupWidget->SetVisibility(false);
		// 设置ItemMesh属性
		ItemMesh->SetSimulatePhysics(false);// 关闭模拟物理
		ItemMesh->SetEnableGravity(false); // 关闭重力
		ItemMesh->SetVisibility(true);	// 可见
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);// 忽略所有通道碰撞
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞
		// 设置球体碰撞
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore); // 响应方式为重叠
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // gpt 问下
		// 设置碰撞Box属性
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_EquipInterping:
		PickupWidget->SetVisibility(false);
		// 设置ItemMesh属性
		ItemMesh->SetSimulatePhysics(false);// 关闭模拟物理
		ItemMesh->SetEnableGravity(false); // 关闭重力
		ItemMesh->SetVisibility(true);	// 可见
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);// 忽略所有通道碰撞
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞
		// 设置球体碰撞
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore); // 响应方式为重叠
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // gpt 问下
		// 设置碰撞Box属性
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		// 设置ItemMesh属性
		ItemMesh->SetSimulatePhysics(true);// 关闭模拟物理
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		ItemMesh->SetEnableGravity(true); // 启用重力
		ItemMesh->SetVisibility(true);
		// 设置球体碰撞
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Overlap); // 响应方式为重叠
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // gpt 问下
		// 设置碰撞Box属性
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;

	case  EItemState::EIS_PickedUp:
		PickupWidget->SetVisibility(false);
		// 设置ItemMesh属性
		ItemMesh->SetSimulatePhysics(false);// 关闭模拟物理
		ItemMesh->SetEnableGravity(false); // 关闭重力
		ItemMesh->SetVisibility(false);	// 可见
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);// 忽略所有通道碰撞
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞
		// 设置球体碰撞
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore); // 响应方式为重叠
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // gpt 问下
		// 设置碰撞Box属性
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AItem::FinishInterping()
{
	bInterping = false;
	if (Character)
	{
		// 完成插入后原有位置ItemCount-1
		Character->IncrementInterpLocItemCount(InterpLocIndex, -1);
		Character->GetPickupItem(this);
		// // 这里考虑要不要，因为那边已经设计了装备完毕的状态，不过考虑有背包，暂时不处理
		// SetItemState(EItemState::EIS_PickedUp);

		Character->UnHighlightInventorySlot();
	}

	// 设置为正常缩放
	SetActorScale3D(FVector::OneVector);

	DisableGlowMaterial();
	bCanChangeCustomDepth = true;
	DisableCustomDepth();

}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;

	if (Character && ItemZCurve)
	{
		// 获取计时器经过时长
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		// 获取对应时间的曲线值  Get curve value corresponding to ElapsedTime
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);
		// 获取开始插值时item的位置  Get the item's initial location when the curve started
		FVector ItemLocation = ItemInterpStartLocation;
		// 获取相机位置 Get location int front of the camera
		const FVector CameraInterpLocation { GetInterpLocation() };

		// 从相机到Item的向量，x和y都为0，只取z， Vector from item to Camera interp location, x and y are zeroed out 
		const FVector ItemToCamera { FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z) };

		// 缩放因数和当前曲率值相乘 Scale factor to multiply with CurveValue
		const float DeltaZ = ItemToCamera.Size();

		const FVector CurrentLocation{ GetActorLocation() };
		// X&Y的插值
		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.f);
		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.f);

		// 设置X&Y到ItemLocation
		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;
		// 向初始位置的Z添加去曲线值，通过DeltaZ缩放
		ItemLocation.Z += CurveValue * DeltaZ;
		// 设置位置
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		// 相机此帧旋转 Camera Rotator this frame
		const FRotator CameraRotation{ Character->GetFollowCamera()->GetComponentRotation().Yaw };
		// 相机旋转+固定偏航 得到Item相对的旋转 Camera rotation plus inital yaw offset 
		FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpInitialYawOffset,0.f };
		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			// 缩放曲率
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			// 缩放
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
		
	}
}

FVector AItem::GetInterpLocation()
{
	if (Character == nullptr) return FVector(0.F);

	switch (ItemType)
	{
		case EItemType::EIT_Ammo:
			return Character->GetInterpLocation(InterpLocIndex).SceneComponent->GetComponentLocation();
			break;
		case EItemType::EIT_Weapon:
			return Character->GetInterpLocation(0).SceneComponent->GetComponentLocation();
			break;
		default:
			return FVector(0.F);
			break;
	}
}

void AItem::PlayPickupSound(bool bForcePlaySound)
{
	if (Character)
	{
		if (bForcePlaySound)
		{
			if (PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, PickupSound);
			}
		}
		else if (Character->ShouldPlayPickupSound())
		{
			Character->StartPickSoundTimer();
			if (PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, PickupSound);
			}
		}
	}
}

void AItem::EnableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(true);
	}
}

void AItem::DisableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(false);
	}
}

void AItem::InitializeCustomDepth()
{
	DisableCustomDepth();
}

void AItem::OnConstruction(const FTransform& Transform)
{
	// 加载表中数据
	// Item稀有度表格路径
	FString RarityTablePath(TEXT("/Script/Engine.DataTable'/Game/_Game/DataTable/ItemRarityDataTable.ItemRarityDataTable'"));

	if (UDataTable* RarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *RarityTablePath)))
	{
		FItemRarityTable* RarityRow = nullptr;

		switch (ItemRarity)
		{
			case EItemRarity::EIR_Damaged:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Damaged"), TEXT(""));
				break;
			case EItemRarity::EIR_Common:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Common"), TEXT(""));
				break;
			case EItemRarity::EIR_UnCommon:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("UnCommon"), TEXT(""));
				break;
			case EItemRarity::EIR_Rare:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Rare"), TEXT(""));
				break;
			case EItemRarity::EIR_Legendary:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Legendary"), TEXT(""));
				break;
			default:
				break;
		}

		if (RarityRow)
		{
			GlowColor = RarityRow->GlowColor;
			LightColor = RarityRow->LightColor;
			DarkColor = RarityRow->DarkColor;
			NumberOfStars = RarityRow->NumberOfStarts;
			IconBackground = RarityRow->IconBackground;
			// 设置自定义深度模板值
			if (GetItemMesh())
			{
				GetItemMesh()->SetCustomDepthStencilValue(RarityRow->CustomDepthStencil);
			}
		}
	}

	if (MaterialInstance)
	{
		// 创建动态材质实例
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		DynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), GlowColor);
		ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
		EnableGlowMaterial();
	}
	
}

void AItem::EnableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0);
	}
}

void AItem::UpdatePulse()
{
	float ElapsedTime {};
	FVector CurveValue {};
	switch (ItemState)
	{
	case EItemState::EIS_Pickup:
		if (PulseCurve)
		{
			// 这样可以拿到定时器已经经过的时长
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
			CurveValue = PulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	case EItemState::EIS_EquipInterping:
		if (InterpPulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
			CurveValue = InterpPulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	}

	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), CurveValue.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelExponent"), CurveValue.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelReflectFraction"), CurveValue.Z * FresnelReflectFraction);
	}
}


void AItem::DisableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 1);
	}
}

void AItem::PlayEquipSound(bool bForcePlaySound)
{
	if (Character)
	{
		if (bForcePlaySound)
		{
			if (EquipSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipSound);
			}
		}
		else if (Character->ShouldEquipSound())
		{
			Character->StartEquipSoundTimer();
			if (EquipSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipSound);
			}
		}
	}
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 装备插值状态下的物体移动插值
	ItemInterp(DeltaTime);

	// 从PulseCurve获取曲线值并设置动态材料参数
	UpdatePulse();
}

void AItem::RestPulseTimer()
{
	StartPulseTimer();
}

void AItem::StartPulseTimer()
{
	if (ItemState == EItemState::EIS_Pickup)
	{
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::RestPulseTimer, PulseCurveTime);
	}
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound)
{
	// 存储角色句柄
	Character = Char;

	// 获取角色插值位置中ItemCount最小的索引
	InterpLocIndex = Character->GetInterpLocationIndex();
	// 增加1
	Character->IncrementInterpLocItemCount(InterpLocIndex, 1);

	PlayPickupSound(bForcePlaySound);

	// 获取Actor位置
	ItemInterpStartLocation = GetActorLocation();

	bInterping = true;
	
	SetItemState(EItemState::EIS_EquipInterping);

	// 清理PickUp下的闪烁定时器
	GetWorldTimerManager().ClearTimer(PulseTimer);

	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);

	// 获得相机初始Yaw偏移
	const double CameraRotationYaw{ Character->GetFollowCamera()->GetComponentRotation().Yaw };
	// 获得Item的初始Yaw
	const double ItemRotationYaw{ GetActorRotation().Yaw };
	// 得到Item和相机之间的偏移
	InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;

	bCanChangeCustomDepth = false;
}

