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

// Deleted FItemBonusStats

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<class USkeletalMesh> EquipmentMesh;

	// AAA GAS Integration: This effect dynamically applies all complex percentages or stat modifications.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats (Server/Backend)")
	TSubclassOf<class UGameplayEffect> EquippedStatEffect;

	// AAA DRY Principle: Removed BonusStats here. Stats are exclusively fetched from the GE CDO.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy", meta = (ClampMin = "0"))
	int32 SellPriceGold = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	// Moved to UI-Only Display
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	// FItemBonusStats BonusStats;
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
