#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "DataAssets/ItemDataAsset.h"
#include "InventoryWidget.generated.h"

// Forward Declarations
class UWrapBox;
class UItemSlotWidget;
class UInventoryComponent;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;

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
	TSubclassOf<UItemSlotWidget> ItemSlotClass;

	// --- Equipment Slots (Knight Online Style) ---
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> HelmSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> NecklaceSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> ChestSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> CloakSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> BeltSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> MainHandSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> OffHandSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> LeftRingSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> RightRingSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> GauntletsSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> LeggingsSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemSlotWidget> BootsSlot;

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
	TObjectPtr<class UTextBlock> CriticalStrikeChanceText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> MovementSpeedText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> AttackDamageText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> SpellDamageText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> CastSpeedText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> ArmorText;

private:
	// Orchestrator method triggered by delegates
	UFUNCTION()
	void RefreshInventoryGrid();

	// Creation helpers (DRY: Separated equipment vs backpack logic)
	void PopulateSlots();
	void InitializeEquipmentSlots();
	void InitializeEquipmentSlot(UItemSlotWidget* SlotWidget, EEquipmentSlot SlotType);
	void EnsureBackpackSlotsCreated();
	void RefreshBackpackSlots();
	
	// AAA: Loop Internal Extractor
	void CreateAndAddBackpackSlot(int32 SlotIndex);
	
	// Stats UI Helper
	void UpdateStatsUI();
	void BindAttributeDelegates();
	void UnbindAttributeDelegates();
	void HandleObservedAttributeChanged(const FOnAttributeChangeData& ChangeData);
	[[nodiscard]] UAbilitySystemComponent* ResolveObservedAbilitySystemComponent() const;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UItemSlotWidget>> BackpackSlotWidgets;

	// AAA Caching: Prevent memory reallocation on every refresh
	TArray<FAttributeDisplayInfo> CachedDisplayMappings;
	TArray<FDelegateHandle> AttributeChangeDelegateHandles;

	// AAA: Called once to build the layout caches
	void BuildCaches();

	// AAA Pure Formatter (SOLID/DRY)
	[[nodiscard]] FString FormatStatValue(float Value, const FString& Label, EStatDisplayFormat Format) const;
};
