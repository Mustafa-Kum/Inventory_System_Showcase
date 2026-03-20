#include "UI/WeaponSlotWidget.h"
#include "DataAssets/WeaponDataAsset.h"
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
		// AAA: Butonun tıklamayı yutmasını (consume) engelliyoruz. 
		// Böylece tıklama Widget'ın kendisine ulaşır ve NativeOnMouseButtonDown (sürükleme) tetiklenir.
		SlotButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		
		// Not: Tıklama özelliğini tamamen kapattığımız için OnClicked artık çalışmayacaktır.
		// User request: "Inventory'de silahı sadece drag yapabiliyor olmamız gerekiyor."
	}
}

void UWeaponSlotWidget::InitializeSlot(UWeaponDataAsset* InWeaponData, int32 InIndex, UInventoryComponent* InInventoryComp, EItemSlotContext InContext)
{
	AssignSlotData(InWeaponData, InIndex, InInventoryComp, InContext);

	if (!WeaponData)
	{
		ClearSlotVisuals();
		return;
	}

	UpdateSlotVisuals();
	SetupTooltipWidget();
}

void UWeaponSlotWidget::AssignSlotData(UWeaponDataAsset* InWeaponData, int32 InIndex, UInventoryComponent* InInventoryComp, EItemSlotContext InContext)
{
	WeaponData = InWeaponData;
	SlotIndex = InIndex;
	InventoryComp = InInventoryComp;
	SlotContext = InContext;
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
		LoadAndSetIconAsync(WeaponData->WeaponData.WeaponIcon);
	}

	if (WeaponNameText)
	{
		WeaponNameText->SetText(WeaponData->WeaponData.WeaponName);
	}
}

void UWeaponSlotWidget::SetupTooltipWidget()
{
	if (!TooltipClass) return;

	UItemTooltipWidget* TooltipWidget = CreateWidget<UItemTooltipWidget>(this, TooltipClass);
	if (TooltipWidget)
	{
		TooltipWidget->SetupTooltip(WeaponData);
		SetToolTip(TooltipWidget);
	}
}

void UWeaponSlotWidget::LoadAndSetIconAsync(const TSoftObjectPtr<UTexture2D>& IconPtr)
{
	if (IconPtr.IsPending())
	{
		// Async Load
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
		// Immediately set if already mapped
		IconImage->SetBrushFromTexture(IconPtr.Get());
	}
}

void UWeaponSlotWidget::OnSlotButtonClicked()
{
	if (!InventoryComp)
	{
		UE_LOG(LogWeaponSlot, Error, TEXT("Inventory Component reference missing when slot clicked!"));
		return;
	}

	// AAA: Click logic disabled as per user request ("sadece drag yapabiliyor olmamız gerekiyor").
	// Equipping is now handled exclusively via Drag & Drop or specific Hotkeys.
	
	// Optional: Broadcast for parent grid if needed (selection/highlighting)
	OnSlotClicked.Broadcast(SlotIndex);
}

FReply UWeaponSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Attempt to start a drag if we have data and left mouse button is pressed.
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && WeaponData != nullptr)
	{
		// Native AAA way to detect dragging
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UWeaponSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!WeaponData || !InventoryComp) return;

	// Create Payload
	UWeaponDragDropOperation* DragOperation = Cast<UWeaponDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UWeaponDragDropOperation::StaticClass()));
	if (!DragOperation) return;

	DragOperation->PayloadWeapon = WeaponData;
	DragOperation->SourceSlotIndex = SlotIndex;
	DragOperation->SourceContext = SlotContext;
	DragOperation->DefaultDragVisual = CreateDragVisualWidget();

	OutOperation = DragOperation;
}

UWeaponSlotWidget* UWeaponSlotWidget::CreateDragVisualWidget()
{
	TSubclassOf<UUserWidget> VisualClass = DragVisualClass;
	if (!VisualClass)
	{
		VisualClass = GetClass();
	}

	UWeaponSlotWidget* VisualWidget = CreateWidget<UWeaponSlotWidget>(GetOwningPlayer(), VisualClass);
	
	if (VisualWidget)
	{
		VisualWidget->InitializeSlot(WeaponData, SlotIndex, InventoryComp, SlotContext);
		VisualWidget->SetRenderOpacity(0.5f); // "Alphalı" visual as requested
	}
	
	return VisualWidget;
}

bool UWeaponSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// AAA: Explicitly return true to indicate this slot is a valid drop target
	return true;
}

bool UWeaponSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UWeaponDragDropOperation* Payload = Cast<UWeaponDragDropOperation>(InOperation);
	if (!Payload || !InventoryComp) return false;

	// Guard: Dropping on the same slot is a no-op
	if (Payload->SourceContext == SlotContext && Payload->SourceSlotIndex == SlotIndex)
	{
		return false;
	}

	// AAA Dispatch: Route to SRP handlers based on source/target context
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
	InventoryComp->EquipWeaponAtIndex(Payload->SourceSlotIndex);
	return true;
}

bool UWeaponSlotWidget::HandleDropFromEquipmentToInventory(UWeaponDragDropOperation* Payload)
{
	InventoryComp->UnequipCurrentWeapon();
	return true;
}

bool UWeaponSlotWidget::HandleDropBetweenInventorySlots(UWeaponDragDropOperation* Payload)
{
	InventoryComp->SetWeaponAtIndex(WeaponData, Payload->SourceSlotIndex);
	InventoryComp->SetWeaponAtIndex(Payload->PayloadWeapon, SlotIndex);
	return true;
}
