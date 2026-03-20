#include "UI/InventoryWidget.h"
#include "UI/WeaponSlotWidget.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Components/InventoryComponent.h"
#include "Abilities/AttributeSets/CharacterAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "DataAssets/WeaponDataAsset.h"

// Custom Log Category
DEFINE_LOG_CATEGORY_STATIC(LogInventoryUI, Log, All);

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// AAA: Build caches once when the UI is constructed
	BuildCaches();
}

void UInventoryWidget::NativeDestruct()
{
	// AAA Standard: Clean up delegates to prevent dangling pointers / crashes on UI destruction
	if (InventoryComp)
	{
		InventoryComp->OnInventoryUpdated.RemoveDynamic(this, &UInventoryWidget::RefreshInventoryGrid);
	}

	Super::NativeDestruct();
}

void UInventoryWidget::InitializeInventoryUI(UInventoryComponent* InInventoryComp)
{
	if (!InInventoryComp) return;

	InventoryComp = InInventoryComp;

	// AAA Hotfix: BuildCaches requires to run BEFORE the first refresh if AddToViewport hasn't fired NativeConstruct yet.
	BuildCaches();

	// Subscribe to the component's state changes (Observer Pattern / DIP)
	InventoryComp->OnInventoryUpdated.AddDynamic(this, &UInventoryWidget::RefreshInventoryGrid);

	// Initial population right after bind
	RefreshInventoryGrid();
}

void UInventoryWidget::RefreshInventoryGrid()
{
	if (!InventoryGrid)
	{
		UE_LOG(LogInventoryUI, Warning, TEXT("InventoryGrid WrapBox missing from Widget blueprint!"));
		return;
	}

	// AAA Clear Slate
	InventoryGrid->ClearChildren();

	PopulateSlots();
	UpdateStatsUI();
}

void UInventoryWidget::PopulateSlots()
{
	if (!WeaponSlotClass)
	{
		UE_LOG(LogInventoryUI, Error, TEXT("WeaponSlotClass is not set in InventoryWidget. Please assign a BP class."));
		return;
	}

	if (!InventoryComp) return;

	// AAA Orchestrator: Separate equipment panel from backpack grid
	InitializeEquipmentSlots();
	PopulateBackpackGrid();
}

// (Structs moved to header for better template resolving)

void UInventoryWidget::BuildCaches()
{
	if (CachedDisplayMappings.Num() > 0) return; // Prevent double caching

	// Cache Stat Mappings
	CachedDisplayMappings.Empty();
	CachedDisplayMappings.Add(FAttributeDisplayInfo(StrengthText.Get(),       UCharacterAttributeSet::GetStrengthAttribute(),             TEXT("Strength"), EStatDisplayFormat::Integer));
	CachedDisplayMappings.Add(FAttributeDisplayInfo(AgilityText.Get(),        UCharacterAttributeSet::GetAgilityAttribute(),              TEXT("Agility"),  EStatDisplayFormat::Integer));
	CachedDisplayMappings.Add(FAttributeDisplayInfo(IntellectText.Get(),      UCharacterAttributeSet::GetIntellectAttribute(),            TEXT("Intellect"),EStatDisplayFormat::Integer));
	CachedDisplayMappings.Add(FAttributeDisplayInfo(StaminaText.Get(),        UCharacterAttributeSet::GetStaminaAttribute(),              TEXT("Stamina"),  EStatDisplayFormat::Integer));
	CachedDisplayMappings.Add(FAttributeDisplayInfo(CriticalStrikeText.Get(), UCharacterAttributeSet::GetCriticalStrikeChanceAttribute(), TEXT("Crit"),     EStatDisplayFormat::Percentage));
	CachedDisplayMappings.Add(FAttributeDisplayInfo(MovementSpeedText.Get(),  UCharacterAttributeSet::GetMovementSpeedAttribute(),        TEXT("Speed"),    EStatDisplayFormat::Integer));
	CachedDisplayMappings.Add(FAttributeDisplayInfo(BaseDamageText.Get(),     UCharacterAttributeSet::GetAttackDamageAttribute(),         TEXT("Damage"),   EStatDisplayFormat::Integer));
	CachedDisplayMappings.Add(FAttributeDisplayInfo(MagicDamageText.Get(),    UCharacterAttributeSet::GetSpellDamageAttribute(),          TEXT("Spell Dmg"),EStatDisplayFormat::Integer));
	CachedDisplayMappings.Add(FAttributeDisplayInfo(CastSpeedText.Get(),      UCharacterAttributeSet::GetCastSpeedAttribute(),            TEXT("Cast Speed"),EStatDisplayFormat::Decimal));
	CachedDisplayMappings.Add(FAttributeDisplayInfo(ArmorText.Get(),          UCharacterAttributeSet::GetArmorAttribute(),               TEXT("Armor"),    EStatDisplayFormat::Integer));
}

void UInventoryWidget::InitializeEquipmentSlots()
{
	if (!InventoryComp) return;

	// AAA Direct Assignments: Guarantees correct routing regardless of initialization timing
	if (HelmSlot) HelmSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
	if (NecklaceSlot) NecklaceSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
	if (ChestSlot) ChestSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
	if (CloakSlot) CloakSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
	if (BeltSlot) BeltSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);

	// The Main Hand gets the actually equipped weapon
	if (MainHandSlot) MainHandSlot->InitializeSlot(InventoryComp->EquippedWeaponData.Get(), -1, InventoryComp, EItemSlotContext::Equipment);

	if (OffHandSlot) OffHandSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
	if (LeftRingSlot) LeftRingSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
	if (RightRingSlot) RightRingSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
	if (GauntletsSlot) GauntletsSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
	if (LeggingsSlot) LeggingsSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
	if (BootsSlot) BootsSlot->InitializeSlot(nullptr, -1, InventoryComp, EItemSlotContext::Equipment);
}

void UInventoryWidget::PopulateBackpackGrid()
{
	for (int32 i = 0; i < TotalBackpackSlots; ++i)
	{
		UWeaponDataAsset* WeaponData = InventoryComp->WeaponInventory.IsValidIndex(i) ? InventoryComp->WeaponInventory[i] : nullptr;
		CreateAndAddBackpackSlot(WeaponData, i);
	}
}

void UInventoryWidget::CreateAndAddBackpackSlot(UWeaponDataAsset* WeaponData, int32 SlotIndex)
{
	UWeaponSlotWidget* NewSlot = CreateWidget<UWeaponSlotWidget>(GetOwningPlayer(), WeaponSlotClass);
	if (!NewSlot) return;

	NewSlot->InitializeSlot(WeaponData, SlotIndex, InventoryComp, EItemSlotContext::Inventory);
	
	UWrapBoxSlot* WrapBoxSlot = InventoryGrid->AddChildToWrapBox(NewSlot);
	if (WrapBoxSlot)
	{
		// AAA UI: Apply user-requested padding to separate slots slightly
		WrapBoxSlot->SetPadding(FMargin(5.0f));
	}
}

// (Structs moved to header for better template resolving)

void UInventoryWidget::UpdateStatsUI()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OwningPawn);
	if (!ASCInterface || !ASCInterface->GetAbilitySystemComponent()) return;

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();

	for (const FAttributeDisplayInfo& Mapping : CachedDisplayMappings)
	{
		if (UTextBlock* TextBlock = Mapping.TargetText)
		{
			const float Value = ASC->GetNumericAttribute(Mapping.Attribute);
			const FString FinalString = FormatStatValue(Value, Mapping.Label, Mapping.Format);
				
			TextBlock->SetText(FText::FromString(FinalString));
		}
	}
}

FString UInventoryWidget::FormatStatValue(float Value, const FString& Label, EStatDisplayFormat Format) const
{
	switch (Format)
	{
		case EStatDisplayFormat::Percentage:
			return FString::Printf(TEXT("%s: %.1f%%"), *Label, Value);
		case EStatDisplayFormat::Decimal:
			return FString::Printf(TEXT("%s: %.2f"), *Label, Value);
		case EStatDisplayFormat::Integer:
		default:
			return FString::Printf(TEXT("%s: %d"), *Label, FMath::TruncToInt(Value));
	}
}
