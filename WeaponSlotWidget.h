#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAssets/WeaponDataAsset.h"
#include "WeaponSlotWidget.generated.h"

// Forward Declarations (AAA: Decrease compile times)
class UWeaponDataAsset;
class UButton;
class UImage;
class UTextBlock;
class UInventoryComponent;

// AAA Delegate: Decouples UI visually from the component logic 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSlotClicked, int32, SlotIndex);

UCLASS(Abstract)
class WOWCLONE_API UWeaponSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup slot data with explicit context so the slot "knows" what its purpose is (Inventory Grid vs Equipment panel)
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void InitializeSlot(UWeaponDataAsset* InWeaponData, int32 InIndex, class UInventoryComponent* InInventoryComp, EItemSlotContext InContext);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeaponSlotClicked OnSlotClicked;

	// Visual class to spawn and attach to mouse when dragging (AAA standard)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DragDrop")
	TSubclassOf<UUserWidget> DragVisualClass;

	// Tooltip class to show on hover
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip")
	TSubclassOf<class UItemTooltipWidget> TooltipClass;

protected:
	virtual void NativeConstruct() override;
	
	/** AAA Drag & Drop Overrides */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// The actual clickable area
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SlotButton;

	// Visual Representation
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	// Background/Empty State Representation (Optional in UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<class UTexture2D> EmptySlotIcon;

	// Weapon Name (Optional in UI)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponNameText;

private:
	// Button Click Handler
	UFUNCTION()
	void OnSlotButtonClicked();

	// AAA: Encapsulate asynchronous icon loading
	void LoadAndSetIconAsync(const TSoftObjectPtr<class UTexture2D>& IconPtr);

	// Cached References
	UPROPERTY(Transient)
	TObjectPtr<UWeaponDataAsset> WeaponData;

	int32 SlotIndex = -1;
	
	// Stores whether this is an Equipment slot or a Backpack slot
	EItemSlotContext SlotContext;

	// In AAA, UI elements only hold weak or transient references to business logic components
	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComp;

	// Internal Helpers (AAA SOLID)
	void AssignSlotData(UWeaponDataAsset* InWeaponData, int32 InIndex, UInventoryComponent* InInventoryComp, EItemSlotContext InContext);
	void ClearSlotVisuals();
	void UpdateSlotVisuals();
	void SetupTooltipWidget();
	UWeaponSlotWidget* CreateDragVisualWidget();

	bool HandleDropFromInventoryToEquipment(class UWeaponDragDropOperation* Payload);
	bool HandleDropFromEquipmentToInventory(class UWeaponDragDropOperation* Payload);
	bool HandleDropBetweenInventorySlots(class UWeaponDragDropOperation* Payload);
};
