// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "DataAssets/ItemDataAsset.h"
#include "ItemDragDropOperation.generated.h"

UCLASS(BlueprintType)
class WOWCLONE_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	// The item data being dragged
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop Payload", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UItemDataAsset> PayloadItem;

	// The index of the slot it came from (if it came from inventory)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop Payload", meta = (ExposeOnSpawn = "true"))
	int32 SourceSlotIndex = -1;

	// Context indicating whether it was dragged from the backpack or the equipment slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop Payload", meta = (ExposeOnSpawn = "true"))
	EItemSlotContext SourceContext = EItemSlotContext::Inventory;

	// The actual equipment slot the drag started from, when applicable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop Payload", meta = (ExposeOnSpawn = "true"))
	EEquipmentSlot SourceEquipmentSlot = EEquipmentSlot::None;
};
