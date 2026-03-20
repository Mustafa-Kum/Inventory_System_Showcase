// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

/**
 * Weapon Rarity Enum (AAA Standard for coloring/UI)
 */
UENUM(BlueprintType)
enum class EWeaponRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
	Legendary	UMETA(DisplayName = "Legendary")
};

/**
 * AAA Enum to define exactly where an item is located (Inventory vs Equipment)
 */
UENUM(BlueprintType)
enum class EItemSlotContext : uint8
{
	Inventory	UMETA(DisplayName = "Inventory"),
	Equipment	UMETA(DisplayName = "Equipment")
};

/**
 * Weapon Type Enum (AnimGraph: Blend Poses by Enum ile doğru BlendSpace seçimi)
 */
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

/**
 * Bonus stats granted by an item when equipped.
 */
USTRUCT(BlueprintType)
struct FItemBonusStats
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus Stats")
	float BonusStrength = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus Stats")
	float BonusAgility = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus Stats")
	float BonusIntellect = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus Stats")
	float BonusStamina = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus Stats")
	float BonusArmor = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus Stats")
	float BonusCriticalStrikeChance = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus Stats")
	float BonusMovementSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus Stats")
	float BonusMagicDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus Stats")
	float BonusCastSpeed = 0.0f;
};

/**
 * Main Data Structure for Weapons
 */
USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()

	// Weapon Name for UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Info")
	FText WeaponName;

	// Silah tipi (AnimGraph'ta silah tipine göre farklı BlendSpace seçimi için)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Info")
	EWeaponType WeaponType = EWeaponType::Sword;

	// Equip Animation Montage (Plays when taking from holster)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<class UAnimMontage> EquipMontage;
	
	// Unequip Animation Montage (Plays when putting to holster)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<class UAnimMontage> UnequipMontage;

	// The socket on the character's hand to attach this weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
	FName EquipSocketName = FName("WeaponSocket_R");

	// The socket on the character's back/hip to holster this weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
	FName HolsterSocketName = FName("HolsterSocket_Back");

	// Weapon Rarity
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Info")
	EWeaponRarity Rarity = EWeaponRarity::Common;

	// 2D Icon for Inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Info")
	TSoftObjectPtr<class UTexture2D> WeaponIcon;

	// 3D Model for the Character to hold
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<class UStaticMesh> WeaponMesh;

	// Base Damage (Physical damage added to character when equipped)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	// Weapon Cast Speed (Interval in seconds, e.g. 3.64). Lower is slower.
	// Agility will act as a 'Haste' multiplier to reduce this interval.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.1"))
	float WeaponCastSpeed = 2.0f;

	// Bonus attributes granted when this item is equipped
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	FItemBonusStats BonusStats;

	// Sell price in Gold
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy", meta = (ClampMin = "0"))
	int32 SellPriceGold = 0;
};

/**
 * 
 */
UCLASS(BlueprintType)
class WOWCLONE_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// The core data of the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Data")
	FWeaponData WeaponData;

	// Overridden to supply a PrimaryAssetId for async loading/Asset Manager
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
