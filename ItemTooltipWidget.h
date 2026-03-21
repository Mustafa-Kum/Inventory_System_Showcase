// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Margin.h"

// AAA: Forward-declare enums to reduce header coupling (full include in .cpp)
enum class EItemRarity : uint8;
enum class EWeaponType : uint8;

#include "ItemTooltipWidget.generated.h"

class UWeaponDataAsset;
class UStatRowWidget;

/**
 * AAA: Tooltip widget to display core item details on hover.
 */
UCLASS(Abstract)
class WOWCLONE_API UItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UItemTooltipWidget(const FObjectInitializer& ObjectInitializer);

	// Initializes the tooltip with data
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void SetupTooltip(UWeaponDataAsset* ItemData);

protected:
	// The Class to spawn for each bonus stat row
	UPROPERTY(EditDefaultsOnly, Category = "Tooltip|Setup")
	TSubclassOf<UStatRowWidget> StatRowClass;

	// --- Configurable Styling ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip|Styling")
	FSlateFontInfo BonusStatFont;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip|Styling")
	FMargin BonusStatPadding = FMargin(0.0f, 2.0f, 0.0f, 2.0f);

	// UI Bindings
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ItemTypeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> BaseDamageText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> AttackSpeedText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> BonusStatsBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> SellPriceText;

private:
	// --- AAA Orchestration Helpers ---
	void UpdateItemName(UWeaponDataAsset* ItemData);
	void UpdateItemType(UWeaponDataAsset* ItemData);
	void UpdateBaseStats(UWeaponDataAsset* ItemData);
	void UpdateBonusStats(UWeaponDataAsset* ItemData);
	void UpdateEconomy(UWeaponDataAsset* ItemData);

	// Helper to add a stat row dynamically
	void AddStatRow(const FString& Prefix, float Value, const FSlateColor& Color, const FString& Suffix);
	
	// --- AAA Pure Formatters (SOLID/DRY) ---
	[[nodiscard]] FSlateColor GetColorForRarity(EItemRarity Rarity) const;
	[[nodiscard]] FString GetStringForWeaponType(EWeaponType Type) const;
};
