#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataAsset.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
	Legendary	UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class EItemSlotContext : uint8
{
	Inventory	UMETA(DisplayName = "Inventory"),
	Equipment	UMETA(DisplayName = "Equipment")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None		UMETA(DisplayName = "None"),
	Weapon		UMETA(DisplayName = "Weapon"),
	Armor		UMETA(DisplayName = "Armor"),
	Consumable	UMETA(DisplayName = "Consumable"),
	Quest		UMETA(DisplayName = "Quest")
};

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None		UMETA(DisplayName = "None"),
	Helm		UMETA(DisplayName = "Helm"),
	Necklace	UMETA(DisplayName = "Necklace"),
	Chest		UMETA(DisplayName = "Chest"),
	Cloak		UMETA(DisplayName = "Cloak"),
	Belt		UMETA(DisplayName = "Belt"),
	MainHand	UMETA(DisplayName = "Main Hand"),
	OffHand		UMETA(DisplayName = "Off Hand"),
	LeftRing	UMETA(DisplayName = "Left Ring"),
	RightRing	UMETA(DisplayName = "Right Ring"),
	Gauntlets	UMETA(DisplayName = "Gauntlets"),
	Leggings	UMETA(DisplayName = "Leggings"),
	Boots		UMETA(DisplayName = "Boots")
};

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

USTRUCT(BlueprintType)
struct FItemData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Info")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Info")
	EItemType ItemType = EItemType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Info")
	EEquipmentSlot ValidEquipmentSlot = EEquipmentSlot::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Info")
	EItemRarity Rarity = EItemRarity::Common;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Info")
	TSoftObjectPtr<class UTexture2D> ItemIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy", meta = (ClampMin = "0"))
	int32 SellPriceGold = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	FItemBonusStats BonusStats;
};

class UItemDataAsset;

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	FInventoryItem() {}
	FInventoryItem(UItemDataAsset* InItemData, int32 InQuantity) : ItemData(InItemData), Quantity(InQuantity) {}

	bool IsValid() const { return ItemData != nullptr && Quantity > 0; }
};

UCLASS(Abstract, BlueprintType)
class WOWCLONE_API UItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data", meta=(ShowOnlyInnerProperties))
	FItemData ItemData;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
