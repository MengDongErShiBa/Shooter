// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"

#include "ShooterCharacter.h"
#include "Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),
	bAiming(false),
	TIPCharacterYaw(0.f),
	TIPCharacterYawLastFrame(0.f),
	RootYawOffset(0.f),
	Pitch(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.F)),
	YawDelta(0.f),
	RecoilWight(0.f),
	bTurningInPlace(false),
	EquippedWeaponType(EWeaponType::EWT_Max),
	bShouldUseFABRIK(false)
{
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	
	if (ShooterCharacter)
	{
		bCrouching = ShooterCharacter->GetCrouching();
		bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
		bEquipping = ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;
		bShouldUseFABRIK = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied || ShooterCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress;
		// 获取玩家横向速度
		FVector Velocity { ShooterCharacter->GetVelocity() };
		Velocity.Z = 0; //相当于归一化? 去掉玩家Z轴的影响
		Speed = Velocity.Size();

		//玩家是否在空中
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		// 玩家是否在移动  玩家加速度值是否大于0
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		// 获取玩家的瞄准方向
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		// 获取玩家移动方向
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

		// 获取水平方向的旋转差异
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		if (ShooterCharacter->GetVelocity().Size() > 0)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}
		
		// FString MovementOffsetYawMessage = FString::Printf(TEXT("MovementOffsetYaw%f"), MovementOffsetYaw);
		//
		// if (GEngine)
		// {
		// 	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, MovementOffsetYawMessage);
		// }

		// 是否正在瞄准
		bAiming = ShooterCharacter->GetAiming();

		if (bReloading) // 是否在重载
		{
			OffsetState = EOffsetState::EOS_Reloading;
		}
		else if (bIsInAir)
		{
			OffsetState = EOffsetState::EOS_InAir;
		}
		else if (ShooterCharacter->GetAiming())
		{
			OffsetState = EOffsetState::EOS_Aiming;
		}
		else
		{
			OffsetState = EOffsetState::EOS_Hip;
		}

		// 检查角色是否装备有效武器
		if (ShooterCharacter->GetEquippedWeapon())
		{
			EquippedWeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
		}
	}

	// 原地旋转
	TurnInPlace();

	// 身体倾斜计算
	lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	//Super::NativeInitializeAnimation();
	// 尝试获取拥有的Pawn
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;

	// 获得角色瞄准位置，也就是看向的位置
	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;
	
	if (Speed > 0 || bIsInAir) 
	{
		// 玩家正在移动，无需处理
		RootYawOffset = 0.f;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

		const float TIPYawDelta { TIPCharacterYaw - TIPCharacterYawLastFrame };

		// Root Yaw Offset，限制在-180~180
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		// 如果Turning是1
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			bTurningInPlace = true;
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
		
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };
			// 根节点偏航偏移值 > 0 左转|根节点偏航偏移值 > 0 右转
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;
		
			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
			if (ABSRootYawOffset > 90.f)
			{
				const float YawExcess{ ABSRootYawOffset - 90.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		else
		{
			bTurningInPlace = false;
		}

	}

	// 设置后坐力
	if (bTurningInPlace) // 在原地转身 
	{
		if (bReloading || bEquipping) // 重载
		{
			RecoilWight= 1.f;
		}
		else
		{
			RecoilWight = 0.f;
		}
	}
	else // 非原地转身
	{
		if (bCrouching)
		{
			if (bReloading || bEquipping)
			{
				RecoilWight= 1.f;
			}
			else
			{
				RecoilWight = 0.1f;
			}
		}
		else
		{
			if (bAiming || bReloading || bEquipping)
			{
				RecoilWight= 1.f;
			}
			else
			{
				RecoilWight= .1f;
			}
		}
	}
	
}

void UShooterAnimInstance::lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr) return;

	// 通过计算玩家的旋转偏移， 去到偏航更改范围，应用于奔跑动画蓝图
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	const FRotator Delta { UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };
	
	const double Target{ Delta.Yaw / DeltaTime};
	const double Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

	// if (GEngine) 
	// {
	// 	GEngine->AddOnScreenDebugMessage(1, -1, FColor::Cyan, FString::Printf(TEXT("YawDelta:%f"),YawDelta));
	// }
}
