#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR UMETA(DisplayName = "突击步枪"),
	
	EAT_MAX UMETA(DisplayName = "默认最大值")
};
