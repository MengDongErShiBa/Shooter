#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "冲锋枪"),
	EWT_AssaultRifle UMETA(DisplayName = "突击步枪"),
	EWT_Pistol UMETA(DisplayName = "手枪"),

	EWT_Max UMETA(DisplayName = "最大值")
};