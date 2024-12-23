// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AAmmo::AAmmo()
{
	// 构建子弹网格&设置为root
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	AmmoCollisionSphere->SetSphereRadius(50.f);
}

void AAmmo::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap);
	AmmoCollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AAmmo::AAmmo::AmmoSphereEndOverlap);
}

void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);
	switch(State)
	{
	case EItemState::EIS_Pickup: //可拾取状态
		// 设置AmmoMesh属性
		AmmoMesh->SetSimulatePhysics(false);// 关闭模拟物理
		AmmoMesh->SetEnableGravity(false); // 关闭重力
		AmmoMesh->SetVisibility(true);	// 可见
		AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);// 忽略所有通道碰撞
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞
		
		break;
	case EItemState::EIS_Equipped:
		// 设置AmmoMesh属性
		AmmoMesh->SetSimulatePhysics(false);// 关闭模拟物理
		AmmoMesh->SetEnableGravity(false); // 关闭重力
		AmmoMesh->SetVisibility(true);	// 可见
		AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);// 忽略所有通道碰撞
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞
		
		break;
	case EItemState::EIS_EquipInterping:
		// 设置AmmoMesh属性
		AmmoMesh->SetSimulatePhysics(false);// 关闭模拟物理
		AmmoMesh->SetEnableGravity(false); // 关闭重力
		AmmoMesh->SetVisibility(true);	// 可见
		AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);// 忽略所有通道碰撞
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞
		
		break;
	case EItemState::EIS_Falling:
		// 设置AmmoMesh属性
		AmmoMesh->SetSimulatePhysics(true);// 关闭模拟物理
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		AmmoMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		AmmoMesh->SetEnableGravity(true); // 启用重力
		
		break;
	}
}

void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (auto OverlappedCharacter = Cast<AShooterCharacter>(OtherActor))
		{
			StartItemCurve(OverlappedCharacter);
			// 关闭碰撞避免重复触发
			AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AAmmo::AmmoSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Do Nothing
}

void AAmmo::EnableCustomDepth()
{
	// Super::EnableCustomDepth();
	AmmoMesh->SetRenderCustomDepth(true);
}

void AAmmo::DisableCustomDepth()
{
	// Super::DisableCustomDepth();
	AmmoMesh->SetRenderCustomDepth(false);
}
