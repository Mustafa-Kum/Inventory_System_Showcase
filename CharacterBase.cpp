#include "Characters/CharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "DataAssets/WeaponDataAsset.h"
#include "DataAssets/CharacterDataAsset.h"
#include "AbilitySystemComponent.h"
#include "Abilities/AttributeSets/CharacterAttributeSet.h"
#include "WoWCloneGameplayTags.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

ACharacterBase::ACharacterBase()
{
	// Performance optimization: disabling Tick function if not strictly needed.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Ability System Component Kurulumu (GAS)
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	// Character Attribute Set Kurulumu (Stats)
	AttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("AttributeSet"));

	// Weapon Component Kurulumu (AAA: Varsayılan olarak çarpışmaları kapalı tutuyoruz)
	WeaponMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComp"));
	WeaponMeshComp->SetupAttachment(GetMesh()); // Varsayılan olarak kemik belirsiz, dinamik bağlanacak
	
	// Çarpışmaları tamamen kapatıyoruz ki mermi sektirmesin, kamerayı sarsmasın veya karakteri itmesin
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComp->SetGenerateOverlapEvents(false); // Performans için gereksiz tetiklenmeleri önleriz
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Load setup from Data Asset
	InitializeCharacterStats();

	// GAS: Başlangıçta karakterin elinde silah olmadığını (Unarmed) sisteme bildiriyoruz.
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(WoWCloneTags::State_Unarmed);
		AbilitySystemComponent->AddLooseGameplayTag(WoWCloneTags::State_Idle);
	}
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharacterBase::InitializeCharacterStats()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	if (CharacterClassData)
	{
		ApplyStartingStats(CharacterClassData->StartingStats);
	}
	else
	{
		// AAA: Safe Fallback — construct default stats if Data Asset is missing
		FCharacterStartingStats FallbackStats;
		FallbackStats.InitialStrength = 2.0f;
		FallbackStats.InitialAgility = 1.0f;
		FallbackStats.InitialIntellect = 3.0f;
		FallbackStats.InitialStamina = 5.0f;
		FallbackStats.BaseArmor = 10.0f;
		FallbackStats.BaseCriticalStrikeChance = 0.0f;
		FallbackStats.BaseMovementSpeed = 350.0f;
		FallbackStats.BasePhysicalDamage = 10.0f;
		FallbackStats.BaseMagicDamage = 10.0f;
		ApplyStartingStats(FallbackStats);
	}
	
	// Force an update to allow PreAttributeChange scaling to calculate derived base values
	// by simulating an update of the primary stats by +0 (dirty trick but fast).
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetStrengthAttribute(), EGameplayModOp::Additive, 0.0f);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetAgilityAttribute(), EGameplayModOp::Additive, 0.0f);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetIntellectAttribute(), EGameplayModOp::Additive, 0.0f);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, 0.0f);
}

void ACharacterBase::ApplyStartingStats(const FCharacterStartingStats& Stats)
{
	AttributeSet->InitStrength(Stats.InitialStrength);
	AttributeSet->InitAgility(Stats.InitialAgility);
	AttributeSet->InitIntellect(Stats.InitialIntellect);
	AttributeSet->InitStamina(Stats.InitialStamina);
	AttributeSet->InitCriticalStrikeChance(Stats.BaseCriticalStrikeChance);
	AttributeSet->InitMovementSpeed(Stats.BaseMovementSpeed);
	AttributeSet->InitHealth(100.0f);
	AttributeSet->InitMaxHealth(100.0f);
	AttributeSet->InitAttackDamage(Stats.BasePhysicalDamage);
	AttributeSet->InitSpellDamage(Stats.BaseMagicDamage);
	AttributeSet->InitArmor(Stats.BaseArmor);
}

void ACharacterBase::ApplyItemStats(UItemDataAsset* ItemData)
{
	if (!AbilitySystemComponent || !ItemData) return;

	if (UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(ItemData))
	{
		// Set weapon interval as base for haste math (Override — weapon defines the base)
		AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetWeaponBaseIntervalAttribute(), EGameplayModOp::Override, WeaponData->WeaponData.WeaponCastSpeed);
		AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetAttackDamageAttribute(), EGameplayModOp::Additive, WeaponData->WeaponData.BaseDamage);
	}

	ModifyItemStats(ItemData, +1.0f);
}

void ACharacterBase::RemoveItemStats(UItemDataAsset* ItemData)
{
	if (!AbilitySystemComponent || !ItemData) return;

	if (UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(ItemData))
	{
		// Reset weapon interval to 0 (Unarmed — no attack interval in UI/Logic)
		AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetWeaponBaseIntervalAttribute(), EGameplayModOp::Override, 0.0f);
		AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetCastSpeedAttribute(), EGameplayModOp::Override, 0.0f);
		AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetAttackDamageAttribute(), EGameplayModOp::Additive, -WeaponData->WeaponData.BaseDamage);
	}

	ModifyItemStats(ItemData, -1.0f);
}

void ACharacterBase::ModifyItemStats(UItemDataAsset* ItemData, float Sign)
{
	// DRY: Single Source of Truth — adding a new bonus stat requires editing only this method
	const FItemBonusStats& Bonus = ItemData->ItemData.BonusStats;

	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetStrengthAttribute(),              EGameplayModOp::Additive, Sign * Bonus.BonusStrength);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetAgilityAttribute(),               EGameplayModOp::Additive, Sign * Bonus.BonusAgility);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetIntellectAttribute(),             EGameplayModOp::Additive, Sign * Bonus.BonusIntellect);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetStaminaAttribute(),               EGameplayModOp::Additive, Sign * Bonus.BonusStamina);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetArmorAttribute(),                 EGameplayModOp::Additive, Sign * Bonus.BonusArmor);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetCriticalStrikeChanceAttribute(),  EGameplayModOp::Additive, Sign * Bonus.BonusCriticalStrikeChance);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetMovementSpeedAttribute(),         EGameplayModOp::Additive, Sign * Bonus.BonusMovementSpeed);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetSpellDamageAttribute(),           EGameplayModOp::Additive, Sign * Bonus.BonusMagicDamage);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(UCharacterAttributeSet::GetCastSpeedAttribute(),             EGameplayModOp::Additive, Sign * Bonus.BonusCastSpeed);
}

void ACharacterBase::SetPendingWeapon(UWeaponDataAsset* InitialWeaponData)
{
	if (!InitialWeaponData) return;

	CurrentWeaponData = InitialWeaponData;
	
	// AAA Standard: Senkron yerine asenkron (arka planda) model yüklemesi
	if (CurrentWeaponData->WeaponData.WeaponMesh.IsPending())
	{
		FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ACharacterBase::OnWeaponMeshLoaded, CurrentWeaponData.Get());
		UAssetManager::GetStreamableManager().RequestAsyncLoad(CurrentWeaponData->WeaponData.WeaponMesh.ToSoftObjectPath(), Delegate);
	}
	else if (CurrentWeaponData->WeaponData.WeaponMesh.IsValid())
	{
		OnWeaponMeshLoaded(CurrentWeaponData);
	}
}

void ACharacterBase::OnWeaponMeshLoaded(UWeaponDataAsset* LoadedWeaponData)
{
	if (IsWeaponLoadValid(LoadedWeaponData))
	{
		ApplyWeaponMesh(LoadedWeaponData);
		SnapWeaponToInitialSocket(LoadedWeaponData);
	}
}

bool ACharacterBase::IsWeaponLoadValid(UWeaponDataAsset* LoadedWeaponData) const
{
	// Eğer yükleme bitene kadar karakter başka bir silah seçtiyse (veya nil olduysa) iptal et
	return LoadedWeaponData && CurrentWeaponData == LoadedWeaponData;
}

void ACharacterBase::ApplyWeaponMesh(UWeaponDataAsset* LoadedWeaponData)
{
	if (WeaponMeshComp && LoadedWeaponData->WeaponData.WeaponMesh.IsValid())
	{
		WeaponMeshComp->SetStaticMesh(LoadedWeaponData->WeaponData.WeaponMesh.Get());
	}
}

void ACharacterBase::SnapWeaponToInitialSocket(UWeaponDataAsset* LoadedWeaponData)
{
	// Başlangıçta silahı direkt olarak sırtına (Holster) koyalım, daha Equip edilmedi
	AttachWeaponToSocket(LoadedWeaponData->WeaponData.HolsterSocketName);
}

void ACharacterBase::OnWeaponEquipNotify()
{
	if (CanProcessWeaponNotify())
	{
		AttachWeaponToSocket(CurrentWeaponData->WeaponData.EquipSocketName);
		SetArmedState(true);
	}

	OnWeaponActionFinished.Broadcast();
}

void ACharacterBase::OnWeaponUnequipNotify()
{
	// Animasyon o frame'e geldiğinde (Kılıç sırta girdiği an) çalışır
	if (CanProcessWeaponNotify())
	{
		AttachWeaponToSocket(CurrentWeaponData->WeaponData.HolsterSocketName);
		SetArmedState(false);
	}

	OnWeaponActionFinished.Broadcast();
}

void ACharacterBase::ClearWeaponMesh()
{
	if (WeaponMeshComp)
	{
		WeaponMeshComp->SetStaticMesh(nullptr);
	}
	CurrentWeaponData = nullptr;
}

// ==============================================================================
// DRY Helpers — Single Source of Truth (AAA Clean Code)
// ==============================================================================

bool ACharacterBase::CanProcessWeaponNotify() const
{
	return CurrentWeaponData != nullptr && WeaponMeshComp != nullptr;
}

void ACharacterBase::AttachWeaponToSocket(FName SocketName)
{
	WeaponMeshComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
}

void ACharacterBase::ToggleGameplayTagPair(UAbilitySystemComponent* ASC, const FGameplayTag& TagToRemove, const FGameplayTag& TagToAdd)
{
	if (!ASC) return;

	ASC->RemoveLooseGameplayTag(TagToRemove);
	ASC->AddLooseGameplayTag(TagToAdd);
}

void ACharacterBase::SetArmedState(bool bIsArmed)
{
	const FGameplayTag& TagToRemove = bIsArmed ? WoWCloneTags::State_Unarmed : WoWCloneTags::State_Armed;
	const FGameplayTag& TagToAdd = bIsArmed ? WoWCloneTags::State_Armed : WoWCloneTags::State_Unarmed;

	ToggleGameplayTagPair(AbilitySystemComponent, TagToRemove, TagToAdd);
}

bool ACharacterBase::HasWeaponEquipped() const
{
	if (!WeaponMeshComp || !CurrentWeaponData) return false;

	// Kılıcın karakterin mesh üzerindeki hangi sokete bağlı olduğuna bakıyoruz.
	// Bu sayede ekstra bir "bIsEquipped" boolean tutmadan fiziksel gerçeği sorgulamış oluyoruz (Single Source of Truth).
	return WeaponMeshComp->GetAttachSocketName() == CurrentWeaponData->WeaponData.EquipSocketName;
}

EWeaponType ACharacterBase::GetEquippedWeaponType() const
{
	if (CurrentWeaponData && HasWeaponEquipped())
	{
		return CurrentWeaponData->WeaponData.WeaponType;
	}
	return EWeaponType::None;
}