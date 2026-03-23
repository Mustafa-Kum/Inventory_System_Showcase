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

	// Tooltip implementation left as is (assume it works with ItemData)
	UItemTooltipWidget* TooltipWidget = CreateWidget<UItemTooltipWidget>(this, TooltipClass);
	if (TooltipWidget)
	{
		TooltipWidget->SetupTooltip(ItemData);
		SetToolTip(TooltipWidget);
	}
}

void UItemSlotWidget::LoadAndSetIconAsync(const TSoftObjectPtr<UTexture2D>& IconPtr)
{
	if (IconPtr.IsPending())
	{
		FStreamableDelegate Delegate = FStreamableDelegate::CreateWeakLambda(this, [this, IconPtr]()
		{
			if (IconImage && IconPtr.IsValid())
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

void UItemSlotWidget::OnSlotButtonClicked()
{
	// Reserved for future use (e.g., selection highlight)
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

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && InventoryComp)
	{
		if (SlotContext == EItemSlotContext::Inventory)
		{
			if (ItemData->ItemData.ItemType == EItemType::Consumable)
			{
				InventoryComp->ConsumeItemAtIndex(SlotIndex);
			}
			else
			{
				InventoryComp->EquipItemAtIndex(SlotIndex, ItemData->ItemData.ValidEquipmentSlot);
			}
			return FReply::Handled();
		}
		else if (SlotContext == EItemSlotContext::Equipment)
		{
			// Right click an equipped item to unequip it
			InventoryComp->UnequipItem(EquipmentSlot);
			return FReply::Handled();
		}
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
	return true;
}

bool UItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UItemDragDropOperation* Payload = Cast<UItemDragDropOperation>(InOperation);
	if (!Payload || !InventoryComp) return false;

	if (Payload->SourceContext == SlotContext && Payload->SourceSlotIndex == SlotIndex)
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

bool UItemSlotWidget::HandleDropFromInventoryToEquipment(UItemDragDropOperation* Payload)
{
	// AAA Drag-Drop VALIDATION! Ensure item fits the slot!
	if (Payload->PayloadItem && Payload->PayloadItem->ItemData.ValidEquipmentSlot == EquipmentSlot)
	{
		InventoryComp->EquipItemAtIndex(Payload->SourceSlotIndex, EquipmentSlot);
		return true;
	}
	else
	{
		UE_LOG(LogItemSlot, Warning, TEXT("Drag/Drop Validation Failed! Item does not fit this slot."));
		return false;
	}
}

bool UItemSlotWidget::HandleDropFromEquipmentToInventory(UItemDragDropOperation* Payload)
{
	InventoryComp->UnequipItemToSlot(Payload->PayloadItem->ItemData.ValidEquipmentSlot, SlotIndex);
	return true;
}

bool UItemSlotWidget::HandleDropBetweenInventorySlots(UItemDragDropOperation* Payload)
{
	InventoryComp->SetItemAtIndex(ItemData, 1, Payload->SourceSlotIndex);
	InventoryComp->SetItemAtIndex(Payload->PayloadItem, 1, SlotIndex);
	return true;
}
