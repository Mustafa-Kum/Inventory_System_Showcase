#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "InventoryWidget.generated.h"

// Forward Declarations
class UWrapBox;
class UWeaponSlotWidget;
class UInventoryComponent;
class UWeaponDataAsset;

/**
 * AAA: Helper struct for equipment slot initialization
 */
USTRUCT(BlueprintType)
struct FEquipmentSlotEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	class UWeaponSlotWidget* Slot = nullptr;

	UPROPERTY(BlueprintReadOnly)
	class UWeaponDataAsset* Data = nullptr;

	FEquipmentSlotEntry() {}
	FEquipmentSlotEntry(class UWeaponSlotWidget* InSlot, class UWeaponDataAsset* InData) : Slot(InSlot), Data(InData) {}
};

/**
 * AAA: Format specifier for stat display — replaces brittle string-matching logic
 */
UENUM()
enum class EStatDisplayFormat : uint8
{
	Integer,		// "Strength: 42"
	Percentage,		// "Crit: 5.0%"
	Decimal			// "Cast Speed: 2.35"
};

/**
 * AAA: Helper struct for attribute display mapping
 */
USTRUCT(BlueprintType)
struct FAttributeDisplayInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	class UTextBlock* TargetText = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttribute Attribute;

	UPROPERTY(BlueprintReadOnly)
	FString Label;

	EStatDisplayFormat Format = EStatDisplayFormat::Integer;

	FAttributeDisplayInfo() {}
	FAttributeDisplayInfo(class UTextBlock* InText, FGameplayAttribute InAttr, FString InLabel, EStatDisplayFormat InFormat)
		: TargetText(InText), Attribute(InAttr), Label(InLabel), Format(InFormat) {}
};

UCLASS(Abstract)
class WOWCLONE_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Call this from PlayerController or Character to hook up data and initial populate
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void InitializeInventoryUI(UInventoryComponent* InInventoryComp);

protected:
	virtual void NativeConstruct() override;

	// AAA Event Unbinding to prevent dangling pointers
	virtual void NativeDestruct() override;

	// Configurable visual structure class for child slots
	UPROPERTY(EditDefaultsOnly, Category = "Inventory UI|Classes")
	TSubclassOf<UWeaponSlotWidget> WeaponSlotClass;

	// --- Equipment Slots (Knight Online Style) ---
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> HelmSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> NecklaceSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> ChestSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> CloakSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> BeltSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> MainHandSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> OffHandSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> LeftRingSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> RightRingSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> GauntletsSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> LeggingsSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> BootsSlot;

	// --- Layout Placeholders ---

	// The container that holds the spawned slots for the backpack
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> InventoryGrid;

	// Placeholder for the 3D Character Preview image/renderer
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> CharacterPreview;

	// Placeholder for displaying current character weight
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> WeightText;

	// Placeholder for displaying player currency (Gold/Coins)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> CurrencyText;

	// --- CHARACTER STATS UI BINDINGS ---
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> StrengthText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> AgilityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> IntellectText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> StaminaText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> CriticalStrikeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> MovementSpeedText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> BaseDamageText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> MagicDamageText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> CastSpeedText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> ArmorText;

	// Number of fixed slots in the backpack (Standard MMORPG grid)
	UPROPERTY(EditDefaultsOnly, Category = "Inventory UI|Setup")
	int32 TotalBackpackSlots = 20;

private:
	// Orchestrator method triggered by delegates
	UFUNCTION()
	void RefreshInventoryGrid();

	// Creation helpers (DRY: Separated equipment vs backpack logic)
	void PopulateSlots();
	void InitializeEquipmentSlots();
	void PopulateBackpackGrid();
	
	// AAA: Loop Internal Extractor
	void CreateAndAddBackpackSlot(UWeaponDataAsset* WeaponData, int32 SlotIndex);
	
	// Stats UI Helper
	void UpdateStatsUI();

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComp;

	// AAA Caching: Prevent memory reallocation on every refresh
	TArray<FAttributeDisplayInfo> CachedDisplayMappings;

	// AAA: Called once to build the layout caches
	void BuildCaches();

	// AAA Pure Formatter (SOLID/DRY)
	[[nodiscard]] FString FormatStatValue(float Value, const FString& Label, EStatDisplayFormat Format) const;
};
