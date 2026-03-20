#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/WeaponDataAsset.h"
#include "InventoryComponent.generated.h"

// Forward Declarations (AAA: Minimize header coupling)
class ACharacterBase;
class AWeaponBase;

// --- WEAPON EQUIP STATE MACHINE ---
// Prevents double-press, race conditions, and state desync during equip/unequip animations
UENUM(BlueprintType)
enum class EWeaponEquipState : uint8
{
	Idle			UMETA(DisplayName = "Idle"),			// No weapon action in progress (ready for input)
	Equipping		UMETA(DisplayName = "Equipping"),		// Playing equip animation
	Unequipping		UMETA(DisplayName = "Unequipping")		// Playing unequip animation
};

// Delegate used to notify the UI when the inventory changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOWCLONE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	/** --- CORE DATA --- */

	// Initial weapon added to bag at start
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UWeaponDataAsset> DefaultStartingWeapon;

	// Actual inventory storage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<TObjectPtr<UWeaponDataAsset>> WeaponInventory;

	// Total backpack size
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 MaxInventorySlots = 20;

	// Currently active weapon data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UWeaponDataAsset> EquippedWeaponData;

	// Prevents animation race conditions
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	EWeaponEquipState WeaponEquipState = EWeaponEquipState::Idle;

	/** --- PUBLIC API (Orchestrators) --- */

	// Entry point for adding/moving weapons
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeaponAtIndex(UWeaponDataAsset* Weapon, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddWeapon(UWeaponDataAsset* NewWeapon);

	// High-level equip logic
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void EquipWeaponAtIndex(int32 Index);

	// High-level draw/holster logic
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleDrawHolster();

	// High-level unequip logic
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UnequipCurrentWeapon();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UnequipCurrentWeaponToSlot(int32 TargetIndex);

	/** --- QUERY API (SOLID: Predicates) --- */

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	[[nodiscard]] bool IsWeaponActionInProgress() const { return WeaponEquipState != EWeaponEquipState::Idle; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	[[nodiscard]] bool HasWeaponEquipped() const { return EquippedWeaponData != nullptr; }

	// AAA Utility: Returns the first empty index, or INDEX_NONE if full
	[[nodiscard]] int32 FindEmptySlotIndex() const;

	// Delegate to update UI
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

protected:
	/** --- ASYNC LOADING HANDLERS --- */
	virtual void OnEquipMontageLoaded(class UWeaponDataAsset* WeaponToEquip);
	virtual void OnUnequipMontageLoaded(class UWeaponDataAsset* WeaponToUnequip);

	/** --- STATE MACHINE HELPERS --- */
	void OnWeaponActionCompleted();
	void SetEquipState(EWeaponEquipState NewState);

	/** --- ORCHESTRATION HELPERS (AAA Pattern) --- */
	
	// Validation
	[[nodiscard]] bool CanEquipWeapon(int32 Index) const;
	[[nodiscard]] bool CanUnequipWeapon() const;
	[[nodiscard]] bool CanToggleDrawHolster() const;

	// Processing
	void Internal_ProcessEquipFlow(int32 Index);
	void Internal_ProcessUnequipFlow(int32 TargetIndex);
	void Internal_SwapEquippedWithInventory(int32 Index);
	void Internal_HandleWeaponTransition(UWeaponDataAsset* WeaponToTransition, bool bIsEquipping);
	
	// Mounting/Dismounting Logic (Centralized Stat Management)
	void MountWeaponStats(UWeaponDataAsset* Weapon);
	void DismountWeaponStats(UWeaponDataAsset* Weapon);

	// Execution
	void ExecuteVisualTransition(UWeaponDataAsset* WeaponData, bool bIsEquipping);
	void ExecuteInstantClear();
	void PlayWeaponMontage(UWeaponDataAsset* LoadedData, UWeaponDataAsset* TargetData);

	/** --- AAA UTILITIES (DRY) --- */
	void ProcessMontageLoad(UWeaponDataAsset* WeaponData, const TSoftObjectPtr<UAnimMontage>& MontageToLoad, bool bIsEquipping);
	void HandlePendingMontage(UWeaponDataAsset* WeaponData, const TSoftObjectPtr<UAnimMontage>& MontageToLoad, bool bIsEquipping);
	void HandleValidMontage(UWeaponDataAsset* WeaponData, bool bIsEquipping);
	void HandleMissingMontage(bool bIsEquipping);
	void SnapWeaponWithoutAnimation(bool bIsEquip);

	/** --- FINALIZATION --- */
	void FinalizeEquipAction();
	void FinalizeUnequipAction();

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacterBase> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UWeaponDataAsset> PendingEquipWeaponData;
};
