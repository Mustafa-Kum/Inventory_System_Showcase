#include "Components/InventoryComponent.h"
#include "Characters/CharacterBase.h"
#include "DataAssets/ItemDataAsset.h"
#include "DataAssets/WeaponDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

// Custom Log Category
DEFINE_LOG_CATEGORY_STATIC(LogInventory, Log, All);

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Pre-allocate slots
	InventorySlots.SetNum(MaxInventorySlots);

	OwnerCharacter = Cast<ACharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogInventory, Error, TEXT("InventoryComponent MUST be attached to a CharacterBase!"));
		return;
	}

	OwnerCharacter->OnWeaponActionFinished.AddUObject(this, &UInventoryComponent::OnWeaponActionCompleted);

	if (DefaultStartingItem)
	{
		AddItem(DefaultStartingItem, 1);
		UE_LOG(LogInventory, Log, TEXT("Started with item in bag: %s"), *DefaultStartingItem->ItemData.ItemName.ToString());
	}
}

void UInventoryComponent::SetItemAtIndex(UItemDataAsset* Item, int32 Quantity, int32 Index)
{
	if (InventorySlots.IsValidIndex(Index))
	{
		InventorySlots[Index] = FInventoryItem(Item, Quantity);
		OnInventoryUpdated.Broadcast();
		
		if (Item)
		{
			UE_LOG(LogInventory, Log, TEXT("Moved Item: %s to Slot: %d"), *Item->ItemData.ItemName.ToString(), Index);
		}
	}
}

void UInventoryComponent::AddItem(UItemDataAsset* NewItem, int32 Quantity)
{
	if (!NewItem) return;

	const int32 EmptyIndex = FindEmptySlotIndex();
	
	if (EmptyIndex == INDEX_NONE)
	{
		UE_LOG(LogInventory, Warning, TEXT("Inventory Full! Could not add item: %s"), *NewItem->ItemData.ItemName.ToString());
		return;
	}

	SetItemAtIndex(NewItem, Quantity, EmptyIndex);
}

int32 UInventoryComponent::FindEmptySlotIndex() const
{
	for (int32 i = 0; i < MaxInventorySlots; ++i)
	{
		if (!InventorySlots[i].IsValid())
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void UInventoryComponent::EquipItemAtIndex(int32 Index, EEquipmentSlot TargetSlot)
{
	if (!CanEquipItem(Index, TargetSlot)) return;

	Internal_ProcessEquipFlow(Index, TargetSlot);
}

void UInventoryComponent::Internal_ProcessEquipFlow(int32 Index, EEquipmentSlot TargetSlot)
{
	UItemDataAsset* ItemToEquip = InventorySlots[Index].ItemData;

	// AAA Two-Handed Block Logic
	if (TargetSlot == EEquipmentSlot::MainHand)
	{
		if (UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(ItemToEquip))
		{
			if (WeaponData->WeaponData.bIsTwoHanded && HasItemEquippedAtSlot(EEquipmentSlot::OffHand))
			{
				UnequipItem(EEquipmentSlot::OffHand);
			}
		}
	}
	else if (TargetSlot == EEquipmentSlot::OffHand)
	{
		if (UWeaponDataAsset* MainHandWeapon = Cast<UWeaponDataAsset>(EquippedItems.FindRef(EEquipmentSlot::MainHand)))
		{
			if (MainHandWeapon->WeaponData.bIsTwoHanded)
			{
				UnequipItem(EEquipmentSlot::MainHand);
			}
		}
	}

	if (HasItemEquippedAtSlot(TargetSlot))
	{
		Internal_SwapEquippedWithInventory(Index, TargetSlot);
	}
	else
	{
		EquippedItems.Add(TargetSlot, ItemToEquip);
		SetItemAtIndex(nullptr, 0, Index);
		MountItemStats(ItemToEquip);
	}

	// Visuals (Only if weapon)
	if (UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(EquippedItems[TargetSlot]))
	{
		OwnerCharacter->SetPendingWeapon(WeaponData);
	}
	
	OnInventoryUpdated.Broadcast();
	
	if (EquippedItems[TargetSlot])
	{
		UE_LOG(LogInventory, Log, TEXT("Equipped item: %s to slot %d"), *EquippedItems[TargetSlot]->ItemData.ItemName.ToString(), (int32)TargetSlot);
	}
}

void UInventoryComponent::Internal_SwapEquippedWithInventory(int32 Index, EEquipmentSlot TargetSlot)
{
	UItemDataAsset* ItemToBag = EquippedItems[TargetSlot];
	UItemDataAsset* ItemToHand = InventorySlots[Index].ItemData;
	int32 QtyToHand = InventorySlots[Index].Quantity;

	DismountItemStats(ItemToBag);
	
	SetItemAtIndex(ItemToBag, 1, Index); // Unstacking to bag
	EquippedItems.Add(TargetSlot, ItemToHand);

	MountItemStats(ItemToHand);
}

bool UInventoryComponent::HasItemEquippedAtSlot(EEquipmentSlot Slot) const
{
	return EquippedItems.Contains(Slot) && EquippedItems[Slot] != nullptr;
}

void UInventoryComponent::ToggleDrawHolster()
{
	if (!CanPerformWeaponAction()) return;

	UWeaponDataAsset* MainHandWeapon = Cast<UWeaponDataAsset>(EquippedItems.FindRef(EEquipmentSlot::MainHand));
	if (!MainHandWeapon) return;

	const bool bIsDrawing = !OwnerCharacter->HasWeaponEquipped();
	Internal_HandleWeaponTransition(MainHandWeapon, bIsDrawing);
}

void UInventoryComponent::Internal_HandleWeaponTransition(UWeaponDataAsset* WeaponToTransition, bool bIsEquipping)
{
	SetEquipState(bIsEquipping ? EWeaponEquipState::Equipping : EWeaponEquipState::Unequipping);
	
	if (bIsEquipping)
	{
		OwnerCharacter->SetPendingWeapon(WeaponToTransition);
		PendingEquipWeaponData = WeaponToTransition;
	}

	ExecuteVisualTransition(WeaponToTransition, bIsEquipping);
}

void UInventoryComponent::UnequipItem(EEquipmentSlot Slot)
{
	// -1 means find first available
	Internal_ProcessUnequipFlow(Slot, -1);
}

void UInventoryComponent::UnequipItemToSlot(EEquipmentSlot Slot, int32 TargetIndex)
{
	Internal_ProcessUnequipFlow(Slot, TargetIndex);
}

void UInventoryComponent::Internal_ProcessUnequipFlow(EEquipmentSlot Slot, int32 TargetIndex)
{
	if (!HasItemEquippedAtSlot(Slot)) return;

	UItemDataAsset* ItemToUnequip = EquippedItems[Slot];
	DismountItemStats(ItemToUnequip);
	
	EquippedItems.Remove(Slot);

	if (InventorySlots.IsValidIndex(TargetIndex) && !InventorySlots[TargetIndex].IsValid())
	{
		SetItemAtIndex(ItemToUnequip, 1, TargetIndex);
	}
	else
	{
		AddItem(ItemToUnequip, 1);
	}

	if (UWeaponDataAsset* WeaponToUnequip = Cast<UWeaponDataAsset>(ItemToUnequip))
	{
		if (OwnerCharacter->HasWeaponEquipped())
		{
			Internal_HandleWeaponTransition(WeaponToUnequip, false);
		}
		else
		{
			ExecuteInstantClear();
		}
	}
	else
	{
		OnInventoryUpdated.Broadcast();
	}
}

void UInventoryComponent::MountItemStats(UItemDataAsset* Item)
{
	if (OwnerCharacter && Item)
	{
		OwnerCharacter->ApplyItemStats(Item);
	}
}

void UInventoryComponent::DismountItemStats(UItemDataAsset* Item)
{
	if (OwnerCharacter && Item)
	{
		OwnerCharacter->RemoveItemStats(Item);
	}
}

void UInventoryComponent::ExecuteVisualTransition(UWeaponDataAsset* WeaponData, bool bIsEquipping)
{
	const TSoftObjectPtr<UAnimMontage>& Montage = bIsEquipping ? WeaponData->WeaponData.EquipMontage : WeaponData->WeaponData.UnequipMontage;
	ProcessMontageLoad(WeaponData, Montage, bIsEquipping);
}

void UInventoryComponent::ExecuteInstantClear()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->ClearWeaponMesh();
	}
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::OnEquipMontageLoaded(UWeaponDataAsset* WeaponToEquip)
{
	PlayWeaponMontage(WeaponToEquip, PendingEquipWeaponData);
}

void UInventoryComponent::OnUnequipMontageLoaded(UWeaponDataAsset* WeaponToUnequip)
{
	PlayWeaponMontage(WeaponToUnequip, WeaponToUnequip);
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->OnWeaponActionFinished.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

void UInventoryComponent::OnWeaponActionCompleted()
{
	const EWeaponEquipState PreviousState = WeaponEquipState;

	if (PreviousState == EWeaponEquipState::Equipping) FinalizeEquipAction();
	else if (PreviousState == EWeaponEquipState::Unequipping) FinalizeUnequipAction();

	SetEquipState(EWeaponEquipState::Idle);
}

void UInventoryComponent::FinalizeEquipAction()
{
	if (PendingEquipWeaponData)
	{
		PendingEquipWeaponData = nullptr;
	}
}

void UInventoryComponent::FinalizeUnequipAction()
{
	// Assuming the map still doesn't have it since we removed it in Internal_ProcessUnequipFlow
	if (!HasItemEquippedAtSlot(EEquipmentSlot::MainHand) && OwnerCharacter)
	{
		OwnerCharacter->ClearWeaponMesh();
	}
}

void UInventoryComponent::SetEquipState(EWeaponEquipState NewState)
{
	WeaponEquipState = NewState;
}

void UInventoryComponent::PlayWeaponMontage(UWeaponDataAsset* LoadedData, UWeaponDataAsset* TargetData)
{
	if (!OwnerCharacter || LoadedData != TargetData) return;

	const TSoftObjectPtr<UAnimMontage> Montage = (WeaponEquipState == EWeaponEquipState::Equipping) 
		? LoadedData->WeaponData.EquipMontage 
		: LoadedData->WeaponData.UnequipMontage;

	if (Montage.IsValid())
	{
		OwnerCharacter->PlayAnimMontage(Montage.Get());
	}
}

bool UInventoryComponent::CanEquipItem(int32 Index, EEquipmentSlot TargetSlot) const
{
	return OwnerCharacter && InventorySlots.IsValidIndex(Index) && InventorySlots[Index].IsValid() && !IsWeaponActionInProgress() && !OwnerCharacter->IsInCombat();
}

bool UInventoryComponent::CanPerformWeaponAction() const
{
	return OwnerCharacter && HasItemEquippedAtSlot(EEquipmentSlot::MainHand) && !IsWeaponActionInProgress() && !OwnerCharacter->IsInCombat();
}

void UInventoryComponent::ProcessMontageLoad(UWeaponDataAsset* WeaponData, const TSoftObjectPtr<UAnimMontage>& MontageToLoad, bool bIsEquipping)
{
	if (MontageToLoad.IsPending()) HandlePendingMontage(WeaponData, MontageToLoad, bIsEquipping);
	else if (MontageToLoad.IsValid()) HandleValidMontage(WeaponData, bIsEquipping);
	else HandleMissingMontage(bIsEquipping);
}

void UInventoryComponent::HandlePendingMontage(UWeaponDataAsset* WeaponData, const TSoftObjectPtr<UAnimMontage>& MontageToLoad, bool bIsEquipping)
{
	FStreamableDelegate Delegate = bIsEquipping 
		? FStreamableDelegate::CreateUObject(this, &UInventoryComponent::OnEquipMontageLoaded, WeaponData)
		: FStreamableDelegate::CreateUObject(this, &UInventoryComponent::OnUnequipMontageLoaded, WeaponData);
		
	UAssetManager::GetStreamableManager().RequestAsyncLoad(MontageToLoad.ToSoftObjectPath(), Delegate);
}

void UInventoryComponent::HandleValidMontage(UWeaponDataAsset* WeaponData, bool bIsEquipping)
{
	bIsEquipping ? OnEquipMontageLoaded(WeaponData) : OnUnequipMontageLoaded(WeaponData);
}

void UInventoryComponent::HandleMissingMontage(bool bIsEquipping)
{
	SnapWeaponWithoutAnimation(bIsEquipping);
}

void UInventoryComponent::SnapWeaponWithoutAnimation(bool bIsEquipping)
{
	if (!OwnerCharacter) return;

	bIsEquipping ? OwnerCharacter->OnWeaponEquipNotify() : OwnerCharacter->OnWeaponUnequipNotify();
}
