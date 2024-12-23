// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "瞄准"),
	EOS_Hip UMETA(DisplayName = "腰射"),
	EOS_Reloading UMETA(DisplayName = "重载中"),
	EOS_InAir UMETA(DisplayName = "在空中"),

	EOS_MAX UMETA(DisplayName = "默认最大值")
};

class AShooterCharacter;
/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	
	UShooterAnimInstance();
	/**
	 * 更新动画属性
	 * @param DeltaTime 
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	/**
	 * 动画蓝图初始化，在动画蓝图创建实例播放动画之前
	 */
	virtual void NativeInitializeAnimation() override;

protected:
	// 处理原地变量
	void TurnInPlace();

	// 处理奔跑时的身体倾斜 Handle calculations for leaning while running
	void lean(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "ture"))
	AShooterCharacter* ShooterCharacter;

	/**
	 * 移动速度
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "ture"))
	float Speed;

	/**
	 * 是否在空中
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "ture"))
	bool bIsInAir;

	/**
	 * 是否为禁止状态，玩家是否在移动
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "ture"))
	bool bIsAccelerating;

	/**
	 * 横向左右移动,水平偏移
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "ture"))
	float MovementOffsetYaw;

	/**
	 * 在停止移动前的一帧偏移
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "ture"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	// 当前帧的Character Yaw值 用于角色静止或在空中的原地旋转
	float TIPCharacterYaw;

	// 最后一帧（上一帧）的Yaw 用于角色静止或在空中原地旋转
	float TIPCharacterYawLastFrame;

	// 根骨骼旋转
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = true))
	float RootYawOffset;

	// 当前帧的旋转曲线值
	float RotationCurve;

	// 最后一帧的曲线值
	float RotationCurveLastFrame;

	// Pitch 上下  用于目标偏移
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = true))
	float Pitch;

	// 是否在重新装载弹药，重新装载弹药停止目标偏移
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = true))
	bool bReloading;

	// 偏移状态，确定要使用的偏移
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = true))
	EOffsetState OffsetState;

	// 角色此帧偏航
	FRotator CharacterRotation;

	// 角色上一帧偏航
	FRotator CharacterRotationLastFrame;

	// 偏航增量用于奔跑时的身体动画混合
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Lean, meta = (AllowPrivateAccess = "true"));
	float YawDelta;

	// 是否蹲伏  true when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crouching, meta = (AllowPrivateAccess = "true"));
	bool bCrouching;

	// 是否蹲伏  true when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crouching, meta = (AllowPrivateAccess = "true"));
	bool bEquipping;

	// 根据原地转弯和瞄准改变后坐力
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	float RecoilWight;

	// 为真时正在原地转弯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	bool bTurningInPlace;
};
