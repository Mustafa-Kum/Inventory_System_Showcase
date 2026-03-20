#include "Components/InventoryComponent.h"
#include "Characters/CharacterBase.h"
#include "DataAssets/WeaponDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

// Custom Log Category
DEFINE_LOG_CATEGORY_STATIC(LogInventory, Log, All);

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // AAA standard: Turn off tick for inventory
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// AAA Initialization: Pre-allocate the required slots so array operations are safe O(1)
	WeaponInventory.SetNum(MaxInventorySlots);

	// Cache the owner
	OwnerCharacter = Cast<ACharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogInventory, Error, TEXT("InventoryComponent MUST be attached to a CharacterBase!"));
		return;
	}

	// AAA State Machine: Bind to CharacterBase's weapon action finished delegate (DIP principle)
	OwnerCharacter->OnWeaponActionFinished.AddUObject(this, &UInventoryComponent::OnWeaponActionCompleted);

	// Eğer Blueprint üzerinden bir başlangıç silahı seçildiyse:
	if (DefaultStartingWeapon)
	{
		// Sadece çantaya (Inventory) ekle. Artık otomatik olarak sırta takılmıyor, 
		// oyuncunun sürükleyip takması gerekiyor. (User request: "sürükleyip koyduğumuzda takmış olmamız gerekiyor")
		AddWeapon(DefaultStartingWeapon);
		
		UE_LOG(LogInventory, Log, TEXT("Started with weapon in bag: %s"), *DefaultStartingWeapon->WeaponData.WeaponName.ToString());
	}
}

void UInventoryComponent::SetWeaponAtIndex(UWeaponDataAsset* Weapon, int32 Index)
{
	if (WeaponInventory.IsValidIndex(Index))
	{
		WeaponInventory[Index] = Weapon;
		OnInventoryUpdated.Broadcast();
		
		if (Weapon)
		{
			UE_LOG(LogInventory, Log, TEXT("Moved Weapon: %s to Slot: %d"), *Weapon->WeaponData.WeaponName.ToString(), Index);
		}
	}
}

void UInventoryComponent::AddWeapon(UWeaponDataAsset* NewWeapon)
{
	if (!NewWeapon) return;

	const int32 EmptyIndex = FindEmptySlotIndex();
	
	// AAA: Fail-fast orchestrator
	if (EmptyIndex == INDEX_NONE)
	{
		UE_LOG(LogInventory, Warning, TEXT("Inventory Full! Could not add weapon: %s"), *NewWeapon->WeaponData.WeaponName.ToString());
		return;
	}

	SetWeaponAtIndex(NewWeapon, EmptyIndex);
}

int32 UInventoryComponent::FindEmptySlotIndex() const
{
	// AAA logic: Find first empty slot (abstracted loop logic)
	for (int32 i = 0; i < MaxInventorySlots; ++i)
	{
		if (WeaponInventory[i] == nullptr)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void UInventoryComponent::EquipWeaponAtIndex(int32 Index)
{
	if (!CanEquipWeapon(Index)) return;

	Internal_ProcessEquipFlow(Index);
}

void UInventoryComponent::Internal_ProcessEquipFlow(int32 Index)
{
	// 1. Swap existing weapon if necessary
	if (HasWeaponEquipped())
	{
		Internal_SwapEquippedWithInventory(Index);
	}
	else
	{
		// 2. Just move into slot
		EquippedWeaponData = WeaponInventory[Index];
		WeaponInventory[Index] = nullptr;
		MountWeaponStats(EquippedWeaponData);
	}

	// 3. Trigger Visuals (Snapping to back)
	OwnerCharacter->SetPendingWeapon(EquippedWeaponData);
	
	OnInventoryUpdated.Broadcast();
	UE_LOG(LogInventory, Log, TEXT("Equipped weapon: %s"), *EquippedWeaponData->WeaponData.WeaponName.ToString());
}

void UInventoryComponent::Internal_SwapEquippedWithInventory(int32 Index)
{
	UWeaponDataAsset* WeaponToBag = EquippedWeaponData;
	UWeaponDataAsset* WeaponToHand = WeaponInventory[Index];

	DismountWeaponStats(WeaponToBag);
	
	WeaponInventory[Index] = WeaponToBag;
	EquippedWeaponData = WeaponToHand;

	MountWeaponStats(EquippedWeaponData);
}

void UInventoryComponent::ToggleDrawHolster()
{
	if (!CanToggleDrawHolster()) return;

	const bool bIsDrawing = !OwnerCharacter->HasWeaponEquipped();
	Internal_HandleWeaponTransition(EquippedWeaponData, bIsDrawing);
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

void UInventoryComponent::UnequipCurrentWeapon()
{
	if (!CanUnequipWeapon()) return;

	Internal_ProcessUnequipFlow(-1); // -1 means find first available
}

void UInventoryComponent::UnequipCurrentWeaponToSlot(int32 TargetIndex)
{
	if (!CanUnequipWeapon()) return;

	Internal_ProcessUnequipFlow(TargetIndex);
}

void UInventoryComponent::Internal_ProcessUnequipFlow(int32 TargetIndex)
{
	UWeaponDataAsset* WeaponToUnequip = EquippedWeaponData;
	DismountWeaponStats(WeaponToUnequip);
	
	EquippedWeaponData = nullptr;

	// Resolve storage
	if (WeaponInventory.IsValidIndex(TargetIndex) && WeaponInventory[TargetIndex] == nullptr)
	{
		SetWeaponAtIndex(WeaponToUnequip, TargetIndex);
	}
	else
	{
		AddWeapon(WeaponToUnequip);
	}

	// Visuals
	if (OwnerCharacter->HasWeaponEquipped())
	{
		Internal_HandleWeaponTransition(WeaponToUnequip, false);
	}
	else
	{
		ExecuteInstantClear();
	}
}

void UInventoryComponent::MountWeaponStats(UWeaponDataAsset* Weapon)
{
	if (OwnerCharacter && Weapon)
	{
		OwnerCharacter->ApplyWeaponStats(Weapon);
	}
}

void UInventoryComponent::DismountWeaponStats(UWeaponDataAsset* Weapon)
{
	if (OwnerCharacter && Weapon)
	{
		OwnerCharacter->RemoveWeaponStats(Weapon);
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
		EquippedWeaponData = PendingEquipWeaponData;
		PendingEquipWeaponData = nullptr;
	}
}

void UInventoryComponent::FinalizeUnequipAction()
{
	if (!EquippedWeaponData && OwnerCharacter)
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

bool UInventoryComponent::CanEquipWeapon(int32 Index) const
{
	return OwnerCharacter && WeaponInventory.IsValidIndex(Index) && WeaponInventory[Index] && !IsWeaponActionInProgress() && !OwnerCharacter->IsInCombat();
}

bool UInventoryComponent::CanUnequipWeapon() const
{
	return OwnerCharacter && EquippedWeaponData && !IsWeaponActionInProgress() && !OwnerCharacter->IsInCombat();
}

bool UInventoryComponent::CanToggleDrawHolster() const
{
	return OwnerCharacter && EquippedWeaponData && !IsWeaponActionInProgress() && !OwnerCharacter->IsInCombat();
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
	if (OwnerCharacter)
	{
		if (bIsEquipping)
		{
			OwnerCharacter->OnWeaponEquipNotify();
		}
		else
		{
			OwnerCharacter->OnWeaponUnequipNotify();
		}
	}
}
