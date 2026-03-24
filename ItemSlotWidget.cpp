#include "UI/ItemSlotWidget.h"
#include "DataAssets/ItemDataAsset.h"
#include "Components/InventoryComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ItemDragDropOperation.h"
#include "UI/ItemTooltipWidget.h"

// Custom Log Category (AAA Standard)
DEFINE_LOG_CATEGORY_STATIC(LogItemSlot, Log, All);

void UItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotButton)
	{
		SlotButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UItemSlotWidget::InitializeSlot(UItemDataAsset* InItemData, int32 InIndex, UInventoryComponent* InInventoryComp, EItemSlotContext InContext, EEquipmentSlot InEqSlot)
{
	AssignSlotData(InItemData, InIndex, InInventoryComp, InContext, InEqSlot);

	if (!ItemData)
	{
		ClearSlotVisuals();
		return;
	}

	UpdateSlotVisuals();
	SetupTooltipWidget();
}

void UItemSlotWidget::AssignSlotData(UItemDataAsset* InItemData, int32 InIndex, UInventoryComponent* InInventoryComp, EItemSlotContext InContext, EEquipmentSlot InEqSlot)
{
	ItemData = InItemData;
	SlotIndex = InIndex;
	InventoryComp = InInventoryComp;
	SlotContext = InContext;
	EquipmentSlot = InEqSlot;
}

void UItemSlotWidget::ClearSlotVisuals()
{
	PendingIconAsset.Reset();

	if (IconImage)
	{
		IconImage->SetBrushFromTexture(EmptySlotIcon ? EmptySlotIcon : nullptr);
		IconImage->SetColorAndOpacity(EmptySlotIcon ? FLinearColor(1, 1, 1, 1) : FLinearColor(1, 1, 1, 0));
	}
	
	if (ItemNameText)
	{
		ItemNameText->SetText(FText::GetEmpty());
	}

	SetToolTip(nullptr);
}

void UItemSlotWidget::UpdateSlotVisuals()
{
	if (IconImage)
	{
		IconImage->SetColorAndOpacity(FLinearColor::White);
		LoadAndSetIconAsync(ItemData->ItemData.ItemIcon);
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(ItemData->ItemData.ItemName);
	}
}

void UItemSlotWidget::SetupTooltipWidget()
{
	if (!TooltipClass) return;

	if (!TooltipWidgetInstance)
	{
		TooltipWidgetInstance = CreateWidget<UItemTooltipWidget>(this, TooltipClass);
	}

	if (TooltipWidgetInstance)
	{
		TooltipWidgetInstance->SetupTooltip(ItemData);
		SetToolTip(TooltipWidgetInstance);
	}
}

void UItemSlotWidget::LoadAndSetIconAsync(const TSoftObjectPtr<UTexture2D>& IconPtr)
{
	PendingIconAsset = IconPtr;

	if (IconPtr.IsNull())
	{
		if (IconImage)
		{
			IconImage->SetBrushFromTexture(nullptr);
		}
		return;
	}

	if (IconPtr.IsPending())
	{
		FStreamableDelegate Delegate = FStreamableDelegate::CreateWeakLambda(this, [this, IconPtr]()
		{
			if (IconImage && IconPtr.IsValid() && PendingIconAsset == IconPtr && ItemData && ItemData->ItemData.ItemIcon == IconPtr)
			{
				IconImage->SetBrushFromTexture(IconPtr.Get());
			}
		});

		UAssetManager::GetStreamableManager().RequestAsyncLoad(IconPtr.ToSoftObjectPath(), Delegate);
	}
	else if (IconPtr.IsValid() && IconImage)
	{
		IconImage->SetBrushFromTexture(IconPtr.Get());
	}
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (ItemData == nullptr)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && CanProcessRightClick())
	{
		return HandleRightClickAction();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemData || !InventoryComp) return;

	UItemDragDropOperation* DragOperation = Cast<UItemDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UItemDragDropOperation::StaticClass()));
	if (!DragOperation) return;

	// Assign the generalized ItemData so any item (Armor, Consumable, Weapon) can be dragged
	DragOperation->PayloadItem = ItemData; 
	DragOperation->SourceSlotIndex = SlotIndex;
	DragOperation->SourceContext = SlotContext;
	DragOperation->SourceEquipmentSlot = EquipmentSlot;
	DragOperation->DefaultDragVisual = CreateDragVisualWidget();

	OutOperation = DragOperation;
}

UItemSlotWidget* UItemSlotWidget::CreateDragVisualWidget()
{
	TSubclassOf<UUserWidget> VisualClass = DragVisualClass;
	if (!VisualClass) VisualClass = GetClass();

	UItemSlotWidget* VisualWidget = CreateWidget<UItemSlotWidget>(GetOwningPlayer(), VisualClass);
	
	if (VisualWidget)
	{
		VisualWidget->InitializeSlot(ItemData, SlotIndex, InventoryComp, SlotContext, EquipmentSlot);
		VisualWidget->SetRenderOpacity(0.5f);
	}
	
	return VisualWidget;
}

bool UItemSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return CanHandleDropPayload(Cast<UItemDragDropOperation>(InOperation));
}

bool UItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UItemDragDropOperation* Payload = Cast<UItemDragDropOperation>(InOperation);
	if (!CanHandleDropPayload(Payload))
	{
		return false;
	}

	if (SlotContext == EItemSlotContext::Equipment && Payload->SourceContext == EItemSlotContext::Inventory)
	{
		return HandleDropFromInventoryToEquipment(Payload);
	}

	if (SlotContext == EItemSlotContext::Inventory && Payload->SourceContext == EItemSlotContext::Equipment)
	{
		return HandleDropFromEquipmentToInventory(Payload);
	}

	if (SlotContext == EItemSlotContext::Inventory && Payload->SourceContext == EItemSlotContext::Inventory)
	{
		return HandleDropBetweenInventorySlots(Payload);
	}

	return false;
}

bool UItemSlotWidget::CanProcessRightClick() const
{
	return ItemData != nullptr && InventoryComp != nullptr;
}

FReply UItemSlotWidget::HandleRightClickAction()
{
	if (SlotContext == EItemSlotContext::Inventory)
	{
		return HandleInventoryRightClickAction();
	}

	if (SlotContext == EItemSlotContext::Equipment)
	{
		return HandleEquipmentRightClickAction();
	}

	return FReply::Unhandled();
}

FReply UItemSlotWidget::HandleInventoryRightClickAction()
{
	if (ItemData->ItemData.ItemType == EItemType::Consumable)
	{
		return InventoryComp->ConsumeItemAtIndex(SlotIndex) ? FReply::Handled() : FReply::Unhandled();
	}

	if (ItemData->ItemData.ValidEquipmentSlot != EEquipmentSlot::None)
	{
		return InventoryComp->EquipItemAtIndex(SlotIndex, ItemData->ItemData.ValidEquipmentSlot) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

FReply UItemSlotWidget::HandleEquipmentRightClickAction()
{
	return InventoryComp->UnequipItem(EquipmentSlot) ? FReply::Handled() : FReply::Unhandled();
}

bool UItemSlotWidget::CanHandleDropPayload(const UItemDragDropOperation* Payload) const
{
	if (!Payload || !InventoryComp || IsDropFromSameSource(Payload))
	{
		return false;
	}

	if (SlotContext == EItemSlotContext::Equipment && Payload->SourceContext == EItemSlotContext::Inventory)
	{
		return Payload->PayloadItem && Payload->PayloadItem->ItemData.ValidEquipmentSlot == EquipmentSlot;
	}

	if (SlotContext == EItemSlotContext::Inventory && Payload->SourceContext == EItemSlotContext::Equipment)
	{
		return SlotIndex != INDEX_NONE && InventoryComp->GetItemAtIndex(SlotIndex) == nullptr;
	}

	if (SlotContext == EItemSlotContext::Inventory && Payload->SourceContext == EItemSlotContext::Inventory)
	{
		return SlotIndex != INDEX_NONE;
	}

	return false;
}

bool UItemSlotWidget::IsDropFromSameSource(const UItemDragDropOperation* Payload) const
{
	return Payload && Payload->SourceContext == SlotContext && Payload->SourceSlotIndex == SlotIndex;
}

bool UItemSlotWidget::HandleDropFromInventoryToEquipment(UItemDragDropOperation* Payload)
{
	// AAA Drag-Drop VALIDATION! Ensure item fits the slot!
	if (Payload->PayloadItem && Payload->PayloadItem->ItemData.ValidEquipmentSlot == EquipmentSlot)
	{
		return InventoryComp->EquipItemAtIndex(Payload->SourceSlotIndex, EquipmentSlot);
	}
	else
	{
		UE_LOG(LogItemSlot, Warning, TEXT("Drag/Drop Validation Failed! Item does not fit this slot."));
		return false;
	}
}

bool UItemSlotWidget::HandleDropFromEquipmentToInventory(UItemDragDropOperation* Payload)
{
	return InventoryComp->UnequipItemToSlot(Payload->SourceEquipmentSlot, SlotIndex);
}

bool UItemSlotWidget::HandleDropBetweenInventorySlots(UItemDragDropOperation* Payload)
{
	return InventoryComp->SwapInventorySlots(Payload->SourceSlotIndex, SlotIndex);
}
