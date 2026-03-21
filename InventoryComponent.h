#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/ItemDataAsset.h"
#include "DataAssets/WeaponDataAsset.h"
#include "InventoryComponent.generated.h"

// Forward Declarations (AAA: Minimize header coupling)
class ACharacterBase;
class AWeaponBase;

// --- WEAPON EQUIP STATE MACHINE ---
UENUM(BlueprintType)
enum class EWeaponEquipState : uint8
{
	Idle			UMETA(DisplayName = "Idle"),
	Equipping		UMETA(DisplayName = "Equipping"),
	Unequipping		UMETA(DisplayName = "Unequipping")
};

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UItemDataAsset> DefaultStartingItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventoryItem> InventorySlots;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 MaxInventorySlots = 20;

	// AAA RPG Multi-Slot Backend
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TMap<EEquipmentSlot, TObjectPtr<UItemDataAsset>> EquippedItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	EWeaponEquipState WeaponEquipState = EWeaponEquipState::Idle;

	/** --- PUBLIC API --- */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetItemAtIndex(UItemDataAsset* Item, int32 Quantity, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(UItemDataAsset* NewItem, int32 Quantity = 1);

	// Multi-Slot Equip logic
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void EquipItemAtIndex(int32 Index, EEquipmentSlot TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UnequipItem(EEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UnequipItemToSlot(EEquipmentSlot Slot, int32 TargetIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleDrawHolster();

	/** --- QUERY API --- */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	[[nodiscard]] bool IsWeaponActionInProgress() const { return WeaponEquipState != EWeaponEquipState::Idle; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	[[nodiscard]] bool HasItemEquippedAtSlot(EEquipmentSlot Slot) const;

	[[nodiscard]] int32 FindEmptySlotIndex() const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

protected:
	virtual void OnEquipMontageLoaded(class UWeaponDataAsset* WeaponToEquip);
	virtual void OnUnequipMontageLoaded(class UWeaponDataAsset* WeaponToUnequip);

	void OnWeaponActionCompleted();
	void SetEquipState(EWeaponEquipState NewState);
	
	[[nodiscard]] bool CanEquipItem(int32 Index, EEquipmentSlot TargetSlot) const;
	[[nodiscard]] bool CanPerformWeaponAction() const;

	void Internal_ProcessEquipFlow(int32 Index, EEquipmentSlot TargetSlot);
	void Internal_ProcessUnequipFlow(EEquipmentSlot Slot, int32 TargetIndex);
	void Internal_SwapEquippedWithInventory(int32 Index, EEquipmentSlot TargetSlot);
	void Internal_HandleWeaponTransition(UWeaponDataAsset* WeaponToTransition, bool bIsEquipping);
	
	void MountItemStats(UItemDataAsset* Item);
	void DismountItemStats(UItemDataAsset* Item);

	void ExecuteVisualTransition(UWeaponDataAsset* WeaponData, bool bIsEquipping);
	void ExecuteInstantClear();
	void PlayWeaponMontage(UWeaponDataAsset* LoadedData, UWeaponDataAsset* TargetData);

	void ProcessMontageLoad(UWeaponDataAsset* WeaponData, const TSoftObjectPtr<UAnimMontage>& MontageToLoad, bool bIsEquipping);
	void HandlePendingMontage(UWeaponDataAsset* WeaponData, const TSoftObjectPtr<UAnimMontage>& MontageToLoad, bool bIsEquipping);
	void HandleValidMontage(UWeaponDataAsset* WeaponData, bool bIsEquipping);
	void HandleMissingMontage(bool bIsEquipping);
	void SnapWeaponWithoutAnimation(bool bIsEquip);

	void FinalizeEquipAction();
	void FinalizeUnequipAction();

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacterBase> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UWeaponDataAsset> PendingEquipWeaponData;
};
