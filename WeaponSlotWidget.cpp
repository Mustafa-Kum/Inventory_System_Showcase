#include "UI/WeaponSlotWidget.h"
#include "DataAssets/ItemDataAsset.h"
#include "Components/InventoryComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "WeaponDragDropOperation.h"
#include "UI/ItemTooltipWidget.h"

// Custom Log Category (AAA Standard)
DEFINE_LOG_CATEGORY_STATIC(LogWeaponSlot, Log, All);

void UWeaponSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotButton)
	{
		SlotButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UWeaponSlotWidget::InitializeSlot(UItemDataAsset* InItemData, int32 InIndex, UInventoryComponent* InInventoryComp, EItemSlotContext InContext, EEquipmentSlot InEqSlot)
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

void UWeaponSlotWidget::AssignSlotData(UItemDataAsset* InItemData, int32 InIndex, UInventoryComponent* InInventoryComp, EItemSlotContext InContext, EEquipmentSlot InEqSlot)
{
	ItemData = InItemData;
	SlotIndex = InIndex;
	InventoryComp = InInventoryComp;
	SlotContext = InContext;
	EquipmentSlot = InEqSlot;
}

void UWeaponSlotWidget::ClearSlotVisuals()
{
	if (IconImage)
	{
		IconImage->SetBrushFromTexture(EmptySlotIcon ? EmptySlotIcon : nullptr);
		IconImage->SetColorAndOpacity(EmptySlotIcon ? FLinearColor(1, 1, 1, 1) : FLinearColor(1, 1, 1, 0));
	}
	
	if (WeaponNameText)
	{
		WeaponNameText->SetText(FText::GetEmpty());
	}

	SetToolTip(nullptr);
}

void UWeaponSlotWidget::UpdateSlotVisuals()
{
	if (IconImage)
	{
		IconImage->SetColorAndOpacity(FLinearColor::White);
		LoadAndSetIconAsync(ItemData->ItemData.ItemIcon);
	}

	if (WeaponNameText)
	{
		WeaponNameText->SetText(ItemData->ItemData.ItemName);
	}
}

void UWeaponSlotWidget::SetupTooltipWidget()
{
	if (!TooltipClass) return;

	// Tooltip implementation left as is (assume it works with ItemData)
	UItemTooltipWidget* TooltipWidget = CreateWidget<UItemTooltipWidget>(this, TooltipClass);
	if (TooltipWidget)
	{
		TooltipWidget->SetupTooltip(Cast<UWeaponDataAsset>(ItemData));
		SetToolTip(TooltipWidget);
	}
}

void UWeaponSlotWidget::LoadAndSetIconAsync(const TSoftObjectPtr<UTexture2D>& IconPtr)
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

void UWeaponSlotWidget::OnSlotButtonClicked()
{
	if (!InventoryComp) return;
	OnSlotClicked.Broadcast(SlotIndex);
}

FReply UWeaponSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && ItemData != nullptr)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UWeaponSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemData || !InventoryComp) return;

	UWeaponDragDropOperation* DragOperation = Cast<UWeaponDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UWeaponDragDropOperation::StaticClass()));
	if (!DragOperation) return;

	// In WeaponDragDropOperation, PayloadWeapon needs to be cast correctly or used as is if it hasn't been changed. Let's assume PayloadWeapon is loosely typed or can hold ItemData.
	// Fast hack to prevent changing dragdrop header unless necessary for true generic items.
	DragOperation->PayloadWeapon = Cast<UWeaponDataAsset>(ItemData); 
	DragOperation->SourceSlotIndex = SlotIndex;
	DragOperation->SourceContext = SlotContext;
	DragOperation->DefaultDragVisual = CreateDragVisualWidget();

	OutOperation = DragOperation;
}

UWeaponSlotWidget* UWeaponSlotWidget::CreateDragVisualWidget()
{
	TSubclassOf<UUserWidget> VisualClass = DragVisualClass;
	if (!VisualClass) VisualClass = GetClass();

	UWeaponSlotWidget* VisualWidget = CreateWidget<UWeaponSlotWidget>(GetOwningPlayer(), VisualClass);
	
	if (VisualWidget)
	{
		VisualWidget->InitializeSlot(ItemData, SlotIndex, InventoryComp, SlotContext, EquipmentSlot);
		VisualWidget->SetRenderOpacity(0.5f);
	}
	
	return VisualWidget;
}

bool UWeaponSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return true;
}

bool UWeaponSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UWeaponDragDropOperation* Payload = Cast<UWeaponDragDropOperation>(InOperation);
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

bool UWeaponSlotWidget::HandleDropFromInventoryToEquipment(UWeaponDragDropOperation* Payload)
{
	// AAA Drag-Drop VALIDATION! Ensure item fits the slot!
	if (Payload->PayloadWeapon && Payload->PayloadWeapon->ItemData.ValidEquipmentSlot == EquipmentSlot)
	{
		InventoryComp->EquipItemAtIndex(Payload->SourceSlotIndex, EquipmentSlot);
		return true;
	}
	else
	{
		UE_LOG(LogWeaponSlot, Warning, TEXT("Drag/Drop Validation Failed! Item does not fit this slot."));
		return false;
	}
}

bool UWeaponSlotWidget::HandleDropFromEquipmentToInventory(UWeaponDragDropOperation* Payload)
{
	InventoryComp->UnequipItemToSlot(Payload->PayloadWeapon->ItemData.ValidEquipmentSlot, SlotIndex);
	return true;
}

bool UWeaponSlotWidget::HandleDropBetweenInventorySlots(UWeaponDragDropOperation* Payload)
{
	InventoryComp->SetItemAtIndex(ItemData, 1, Payload->SourceSlotIndex);
	InventoryComp->SetItemAtIndex(Payload->PayloadWeapon, 1, SlotIndex);
	return true;
}
