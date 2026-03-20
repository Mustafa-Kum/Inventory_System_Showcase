#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
// AAA: Forward declare enum to minimize header coupling (avoid pulling entire DataAsset header into every Character include)
enum class EWeaponType : uint8;
#include "CharacterBase.generated.h"

// AAA: Delegate for weapon state machine — CharacterBase broadcasts, InventoryComponent listens (DIP)
DECLARE_MULTICAST_DELEGATE(FOnWeaponActionFinished);

UCLASS(Abstract, Blueprintable)
class WOWCLONE_API ACharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	// IAbilitySystemInterface
	[[nodiscard]] virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// SOLID: Virtual getter for AnimInstance to read multiplier safely without casting to child classes.
	// This makes CharacterBase open for extension but closed for modification.
	[[nodiscard]] virtual float GetBackwardMovementSpeedMultiplier() const { return 1.0f; }

	// AnimInstance'ın combat durumunu okuması için (SOLID: Open/Closed Principle)
	[[nodiscard]] virtual bool IsInCombat() const { return false; }

	// Şu an equipped silahın tipini döner (AnimGraph: Blend Poses by Enum)
	[[nodiscard]] virtual EWeaponType GetEquippedWeaponType() const;

	// Animasyon sisteminin karakterin elinde silah olup olmadığını anlaması için (AAA: Encapsulation)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	[[nodiscard]] bool HasWeaponEquipped() const;

public:
	// --- WEAPON SYSTEM ---

	// Blueprint'ten veya Animation Notify'dan çağrılır: Silahı ele geçir
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void OnWeaponEquipNotify();

	// Blueprint'ten veya Animation Notify'dan çağrılır: Silahı sırta as
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void OnWeaponUnequipNotify();

	// Silahı karakterden tamamen kaldırır (Mesh'i siler ve referansı sıfırlar)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ClearWeaponMesh();

	// --- ITEM STAT MODIFIERS ---
	// Silah takıldığında veya çıkarıldığında statları güncellemek için (AAA Standard)
	virtual void ApplyWeaponStats(class UWeaponDataAsset* WeaponData);
	virtual void RemoveWeaponStats(class UWeaponDataAsset* WeaponData);

	// --- VIRTUAL INPUT HOOKS (AAA: DIP - Decoupling Components from Hero class) ---
	virtual void Move(const FInputActionValue& Value) {}
	virtual void Look(const FInputActionValue& Value) {}
	virtual void RightClickStarted() {}
	virtual void RightClickCompleted() {}
	virtual void LeftClickStarted() {}
	virtual void LeftClickCompleted() {}
	virtual void Zoom(const FInputActionValue& Value) {}
	virtual void ToggleWeapon() {}
	virtual void ToggleWalk() {}
	virtual void ToggleCombat() {}
	virtual void ToggleInventory() {}

	// InventoryComponent tetikler, montaj süresince bekleyeceğimiz hedef silahı kaydeder
	virtual void SetPendingWeapon(class UWeaponDataAsset* InitialWeaponData);

protected:
	// Async Asset Loading Callbacks (AAA Standard)
	virtual void OnWeaponMeshLoaded(class UWeaponDataAsset* LoadedWeaponData);
	
	[[nodiscard]] bool IsWeaponLoadValid(class UWeaponDataAsset* LoadedWeaponData) const;
	void ApplyWeaponMesh(class UWeaponDataAsset* LoadedWeaponData);
	void SnapWeaponToInitialSocket(class UWeaponDataAsset* LoadedWeaponData);

	// AAA DRY: Shared tag-swap utility used by SetArmedState and subclass combat tag toggles
	void ToggleGameplayTagPair(class UAbilitySystemComponent* ASC, const FGameplayTag& TagToRemove, const FGameplayTag& TagToAdd);

private:
	// DRY Helpers: Eliminate duplication between Equip/Unequip notify methods
	[[nodiscard]] bool CanProcessWeaponNotify() const;
	void AttachWeaponToSocket(FName SocketName);
	void SetArmedState(bool bIsArmed);

	// DRY: Shared stat modification — ApplyWeaponStats(+1) / RemoveWeaponStats(-1)
	void ModifyWeaponStats(class UWeaponDataAsset* WeaponData, float Sign);

	// Applies base stats from the assigned CharacterDataAsset
	void InitializeCharacterStats();

	// Ability System Component (GAS)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComponent;

	// Character Attribute Set (Primary, Secondary, Derived Stats)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCharacterAttributeSet> AttributeSet;

	// AAA Data-Driven Setup
	// Sınıfın (Mage, Warrior vb.) başlangıç değerlerini ve yeteneklerini tutan Data Asset
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Setup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCharacterDataAsset> CharacterClassData;

	// Silahı karakterin modelinde tutacağımız Component (AAA Standartı: Her zaman kapalı collision)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> WeaponMeshComp;
	
	// Karakterin şu an hedeflediği veya tuttuğu silahın referansı
	UPROPERTY(Transient)
	TObjectPtr<class UWeaponDataAsset> CurrentWeaponData;

public:
	// --- WEAPON STATE MACHINE DELEGATE ---
	// AnimNotify tamamlandığında broadcast edilir, InventoryComponent state geçişi yapar (AAA: DIP)
	FOnWeaponActionFinished OnWeaponActionFinished;
};