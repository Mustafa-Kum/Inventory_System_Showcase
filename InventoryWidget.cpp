#include "UI/InventoryWidget.h"
#include "UI/ItemSlotWidget.h"
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

// Custom Log Category
DEFINE_LOG_CATEGORY_STATIC(LogInventoryUI, Log, All);

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// AAA: Build caches once when the UI is constructed
	BuildCaches();
	BindAttributeDelegates();
}

void UInventoryWidget::NativeDestruct()
{
	// AAA Standard: Clean up delegates to prevent dangling pointers / crashes on UI destruction
	if (InventoryComp)
	{
		InventoryComp->OnInventoryUpdated.RemoveDynamic(this, &UInventoryWidget::RefreshInventoryGrid);
	}

	UnbindAttributeDelegates();

	Super::NativeDestruct();
}

void UInventoryWidget::InitializeInventoryUI(UInventoryComponent* InInventoryComp)
{
	if (!InInventoryComp) return;

	if (InventoryComp)
	{
		InventoryComp->OnInventoryUpdated.RemoveDynamic(this, &UInventoryWidget::RefreshInventoryGrid);
	}

	InventoryComp = InInventoryComp;

	// AAA Hotfix: BuildCaches requires to run BEFORE the first refresh if AddToViewport hasn't fired NativeConstruct yet.
	BuildCaches();

	// Subscribe to the component's state changes (Observer Pattern / DIP)
	InventoryComp->OnInventoryUpdated.RemoveDynamic(this, &UInventoryWidget::RefreshInventoryGrid);
	InventoryComp->OnInventoryUpdated.AddDynamic(this, &UInventoryWidget::RefreshInventoryGrid);
	BindAttributeDelegates();

	// Initial population right after bind
	RefreshInventoryGrid();
}

void UInventoryWidget::RefreshInventoryGrid()
{
	PopulateSlots();
	UpdateStatsUI();
}

void UInventoryWidget::PopulateSlots()
{
	if (!ItemSlotClass)
	{
		UE_LOG(LogInventoryUI, Error, TEXT("ItemSlotClass is not set in InventoryWidget. Please assign a BP class."));
		return;
	}

	if (!InventoryComp) return;

	// AAA Orchestrator: Separate equipment panel from backpack grid
	InitializeEquipmentSlots();
	EnsureBackpackSlotsCreated();
	RefreshBackpackSlots();
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

void UInventoryWidget::BindAttributeDelegates()
{
	BuildCaches();
	UnbindAttributeDelegates();

	ObservedAbilitySystemComponent = ResolveObservedAbilitySystemComponent();
	if (!ObservedAbilitySystemComponent)
	{
		return;
	}

	AttributeChangeDelegateHandles.Reserve(CachedDisplayMappings.Num());
	for (const FAttributeDisplayInfo& Mapping : CachedDisplayMappings)
	{
		AttributeChangeDelegateHandles.Add(
			ObservedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Mapping.Attribute)
				.AddUObject(this, &UInventoryWidget::HandleObservedAttributeChanged));
	}
}

void UInventoryWidget::UnbindAttributeDelegates()
{
	if (ObservedAbilitySystemComponent)
	{
		const int32 BoundHandleCount = FMath::Min(AttributeChangeDelegateHandles.Num(), CachedDisplayMappings.Num());
		for (int32 Index = 0; Index < BoundHandleCount; ++Index)
		{
			if (AttributeChangeDelegateHandles[Index].IsValid())
			{
				ObservedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedDisplayMappings[Index].Attribute)
					.Remove(AttributeChangeDelegateHandles[Index]);
			}
		}
	}

	AttributeChangeDelegateHandles.Reset();
	ObservedAbilitySystemComponent = nullptr;
}

void UInventoryWidget::HandleObservedAttributeChanged(const FOnAttributeChangeData&)
{
	UpdateStatsUI();
}

UAbilitySystemComponent* UInventoryWidget::ResolveObservedAbilitySystemComponent() const
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OwningPawn);
	return ASCInterface ? ASCInterface->GetAbilitySystemComponent() : nullptr;
}

void UInventoryWidget::InitializeEquipmentSlots()
{
	if (!InventoryComp) return;

	InitializeEquipmentSlot(HelmSlot, EEquipmentSlot::Helm);
	InitializeEquipmentSlot(NecklaceSlot, EEquipmentSlot::Necklace);
	InitializeEquipmentSlot(ChestSlot, EEquipmentSlot::Chest);
	InitializeEquipmentSlot(CloakSlot, EEquipmentSlot::Cloak);
	InitializeEquipmentSlot(BeltSlot, EEquipmentSlot::Belt);
	InitializeEquipmentSlot(MainHandSlot, EEquipmentSlot::MainHand);
	InitializeEquipmentSlot(OffHandSlot, EEquipmentSlot::OffHand);
	InitializeEquipmentSlot(LeftRingSlot, EEquipmentSlot::LeftRing);
	InitializeEquipmentSlot(RightRingSlot, EEquipmentSlot::RightRing);
	InitializeEquipmentSlot(GauntletsSlot, EEquipmentSlot::Gauntlets);
	InitializeEquipmentSlot(LeggingsSlot, EEquipmentSlot::Leggings);
	InitializeEquipmentSlot(BootsSlot, EEquipmentSlot::Boots);
}

void UInventoryWidget::InitializeEquipmentSlot(UItemSlotWidget* SlotWidget, EEquipmentSlot SlotType)
{
	if (!SlotWidget || !InventoryComp)
	{
		return;
	}

	SlotWidget->InitializeSlot(InventoryComp->GetEquippedItem(SlotType), -1, InventoryComp, EItemSlotContext::Equipment, SlotType);
}

void UInventoryWidget::EnsureBackpackSlotsCreated()
{
	if (!InventoryGrid)
	{
		UE_LOG(LogInventoryUI, Warning, TEXT("InventoryGrid WrapBox missing from Widget blueprint!"));
		return;
	}

	if (BackpackSlotWidgets.Num() == TotalBackpackSlots)
	{
		return;
	}

	InventoryGrid->ClearChildren();
	BackpackSlotWidgets.Empty();

	for (int32 i = 0; i < TotalBackpackSlots; ++i)
	{
		CreateAndAddBackpackSlot(i);
	}
}

void UInventoryWidget::RefreshBackpackSlots()
{
	if (!InventoryComp) return;

	for (int32 SlotIndex = 0; SlotIndex < BackpackSlotWidgets.Num(); ++SlotIndex)
	{
		if (UItemSlotWidget* SlotWidget = BackpackSlotWidgets[SlotIndex])
		{
			SlotWidget->InitializeSlot(InventoryComp->GetItemAtIndex(SlotIndex), SlotIndex, InventoryComp, EItemSlotContext::Inventory, EEquipmentSlot::None);
		}
	}
}

void UInventoryWidget::CreateAndAddBackpackSlot(int32 SlotIndex)
{
	UItemSlotWidget* NewSlot = CreateWidget<UItemSlotWidget>(GetOwningPlayer(), ItemSlotClass);
	if (!NewSlot) return;

	NewSlot->InitializeSlot(InventoryComp ? InventoryComp->GetItemAtIndex(SlotIndex) : nullptr, SlotIndex, InventoryComp, EItemSlotContext::Inventory, EEquipmentSlot::None);
	
	UWrapBoxSlot* WrapBoxSlot = InventoryGrid->AddChildToWrapBox(NewSlot);
	if (WrapBoxSlot)
	{
		// AAA UI: Apply user-requested padding to separate slots slightly
		WrapBoxSlot->SetPadding(FMargin(5.0f));
	}

	BackpackSlotWidgets.Add(NewSlot);
}

// (Structs moved to header for better template resolving)

void UInventoryWidget::UpdateStatsUI()
{
	UAbilitySystemComponent* ASC = ObservedAbilitySystemComponent.Get();
	if (!ASC)
	{
		ASC = ResolveObservedAbilitySystemComponent();
		if (!ASC)
		{
			return;
		}

		ObservedAbilitySystemComponent = ASC;
	}

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
