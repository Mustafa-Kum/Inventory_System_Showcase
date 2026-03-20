// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "DataAssets/WeaponDataAsset.h"
#include "WeaponDragDropOperation.generated.h"

UCLASS(BlueprintType)
class WOWCLONE_API UWeaponDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	// The weapon data being dragged
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop Payload", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UWeaponDataAsset> PayloadWeapon;

	// The index of the slot it came from (if it came from inventory)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop Payload", meta = (ExposeOnSpawn = "true"))
	int32 SourceSlotIndex = -1;

	// Context indicating whether it was dragged from the backpack or the equipment slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop Payload", meta = (ExposeOnSpawn = "true"))
	EItemSlotContext SourceContext = EItemSlotContext::Inventory;
};
