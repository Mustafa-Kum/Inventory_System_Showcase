#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAssets/ItemDataAsset.h" // Needed for EEquipmentSlot enum

// AAA: Forward-declare enum to reduce header coupling (full include in .cpp)
enum class EItemSlotContext : uint8;

#include "WeaponSlotWidget.generated.h"

class UItemDataAsset;
class UButton;
class UImage;
class UTextBlock;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSlotClicked, int32, SlotIndex);

UCLASS(Abstract)
class WOWCLONE_API UWeaponSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup slot data with explicit context so the slot "knows" what its purpose is
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void InitializeSlot(UItemDataAsset* InItemData, int32 InIndex, class UInventoryComponent* InInventoryComp, EItemSlotContext InContext, EEquipmentSlot InEqSlot = EEquipmentSlot::None);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeaponSlotClicked OnSlotClicked;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DragDrop")
	TSubclassOf<UUserWidget> DragVisualClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip")
	TSubclassOf<class UItemTooltipWidget> TooltipClass;

protected:
	virtual void NativeConstruct() override;
	
	/** AAA Drag & Drop Overrides */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SlotButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<class UTexture2D> EmptySlotIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponNameText;

private:
	UFUNCTION()
	void OnSlotButtonClicked();

	void LoadAndSetIconAsync(const TSoftObjectPtr<class UTexture2D>& IconPtr);

	UPROPERTY(Transient)
	TObjectPtr<UItemDataAsset> ItemData;

	int32 SlotIndex = -1;
	
	EItemSlotContext SlotContext;
	EEquipmentSlot EquipmentSlot = EEquipmentSlot::None;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComp;

	void AssignSlotData(UItemDataAsset* InItemData, int32 InIndex, UInventoryComponent* InInventoryComp, EItemSlotContext InContext, EEquipmentSlot InEqSlot);
	void ClearSlotVisuals();
	void UpdateSlotVisuals();
	void SetupTooltipWidget();
	UWeaponSlotWidget* CreateDragVisualWidget();

	bool HandleDropFromInventoryToEquipment(class UWeaponDragDropOperation* Payload);
	bool HandleDropFromEquipmentToInventory(class UWeaponDragDropOperation* Payload);
	bool HandleDropBetweenInventorySlots(class UWeaponDragDropOperation* Payload);
};
