#pragma once

#include "CoreMinimal.h"
#include "DataAssets/ItemDataAsset.h"
#include "WeaponDataAsset.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None		UMETA(DisplayName = "None"),
	Sword		UMETA(DisplayName = "Sword"),
	Axe			UMETA(DisplayName = "Axe"),
	Mace		UMETA(DisplayName = "Mace"),
	Staff		UMETA(DisplayName = "Staff"),
	Dagger		UMETA(DisplayName = "Dagger")
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Info")
	EWeaponType WeaponType = EWeaponType::Sword;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<class UAnimMontage> EquipMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<class UAnimMontage> UnequipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
	FName EquipSocketName = FName("WeaponSocket_R");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
	FName HolsterSocketName = FName("HolsterSocket_Back");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<class UStaticMesh> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.1"))
	float WeaponCastSpeed = 2.0f;
};

UCLASS(BlueprintType)
class WOWCLONE_API UWeaponDataAsset : public UItemDataAsset
{
	GENERATED_BODY()
	
public:
	UWeaponDataAsset();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Data", meta=(ShowOnlyInnerProperties))
	FWeaponData WeaponData;
};
