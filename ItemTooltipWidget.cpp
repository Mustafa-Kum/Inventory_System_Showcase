#include "UI/ItemTooltipWidget.h"
#include "DataAssets/WeaponDataAsset.h"
#include "UI/StatRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Font.h"

UItemTooltipWidget::UItemTooltipWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// AAA: Load default engine font so the Inspector doesn't show 'None'
	static ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(TEXT("/Engine/EngineFonts/Roboto"));
	if (RobotoFontObj.Succeeded())
	{
		BonusStatFont.FontObject = RobotoFontObj.Object;
		BonusStatFont.TypefaceFontName = FName("Regular");
	}
	BonusStatFont.Size = 12;

	BonusStatPadding = FMargin(0.0f, 2.0f, 0.0f, 2.0f);
}

void UItemTooltipWidget::SetupTooltip(UWeaponDataAsset* ItemData)
{
	// AAA: Fail-fast Validation
	if (!ItemData) return;

	// AAA: High-Level Orchestration
	UpdateItemName(ItemData);
	UpdateItemType(ItemData);
	UpdateBaseStats(ItemData);
	UpdateBonusStats(ItemData);
	UpdateEconomy(ItemData);
}

void UItemTooltipWidget::UpdateItemName(UWeaponDataAsset* ItemData)
{
	if (!ItemNameText) return;

	ItemNameText->SetText(ItemData->ItemData.ItemName);
	ItemNameText->SetColorAndOpacity(GetColorForRarity(ItemData->ItemData.Rarity));
}

void UItemTooltipWidget::UpdateItemType(UWeaponDataAsset* ItemData)
{
	if (!ItemTypeText) return;

	const FString TypeStr = GetStringForWeaponType(ItemData->WeaponData.WeaponType);
	ItemTypeText->SetText(FText::FromString(TypeStr));
}

void UItemTooltipWidget::UpdateBaseStats(UWeaponDataAsset* ItemData)
{
	if (BaseDamageText)
	{
		BaseDamageText->SetText(FText::Format(NSLOCTEXT("Tooltip", "DamageFmt", "{0} Damage"), FText::AsNumber(FMath::RoundToInt(ItemData->WeaponData.BaseDamage))));
	}

	if (AttackSpeedText)
	{
		AttackSpeedText->SetText(FText::Format(NSLOCTEXT("Tooltip", "SpeedFmt", "Speed {0}"), FText::AsNumber(ItemData->WeaponData.WeaponCastSpeed)));
	}
}

void UItemTooltipWidget::UpdateBonusStats(UWeaponDataAsset* ItemData)
{
	if (!BonusStatsBox || !StatRowClass) return;

	BonusStatsBox->ClearChildren();
	const FSlateColor GreenColor = FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f));
	const FItemBonusStats& Stats = ItemData->ItemData.BonusStats;
	
	if (Stats.BonusStamina > 0) AddStatRow(TEXT("+"), Stats.BonusStamina, GreenColor, TEXT(" Stamina"));
	if (Stats.BonusStrength > 0) AddStatRow(TEXT("+"), Stats.BonusStrength, GreenColor, TEXT(" Strength")); 
	if (Stats.BonusAgility > 0) AddStatRow(TEXT("+"), Stats.BonusAgility, GreenColor, TEXT(" Agility"));
	if (Stats.BonusIntellect > 0) AddStatRow(TEXT("+"), Stats.BonusIntellect, GreenColor, TEXT(" Intellect"));
	if (Stats.BonusArmor > 0) AddStatRow(TEXT("+"), Stats.BonusArmor, GreenColor, TEXT(" Armor"));
	if (Stats.BonusCriticalStrikeChance > 0) AddStatRow(TEXT("+"), Stats.BonusCriticalStrikeChance, GreenColor, TEXT("% Critical Strike"));
	if (Stats.BonusMovementSpeed > 0) AddStatRow(TEXT("+"), Stats.BonusMovementSpeed, GreenColor, TEXT(" Movement Speed"));
	if (Stats.BonusMagicDamage > 0) AddStatRow(TEXT("+"), Stats.BonusMagicDamage, GreenColor, TEXT(" Magic Damage"));
	if (Stats.BonusCastSpeed > 0) AddStatRow(TEXT("+"), Stats.BonusCastSpeed, GreenColor, TEXT(" Cast Speed"));
}

void UItemTooltipWidget::UpdateEconomy(UWeaponDataAsset* ItemData)
{
	if (!SellPriceText) return;

	if (ItemData->ItemData.SellPriceGold > 0)
	{
		SellPriceText->SetText(FText::Format(NSLOCTEXT("Tooltip", "PriceFmt", "Sell Price: {0} Gold"), FText::AsNumber(ItemData->ItemData.SellPriceGold)));
		SellPriceText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SellPriceText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FSlateColor UItemTooltipWidget::GetColorForRarity(EItemRarity Rarity) const
{
	switch (Rarity)
	{
		case EItemRarity::Uncommon:	return FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f)); // Green
		case EItemRarity::Rare:		return FSlateColor(FLinearColor(0.0f, 0.4f, 1.0f)); // Blue
		case EItemRarity::Epic:		return FSlateColor(FLinearColor(0.6f, 0.2f, 0.8f)); // Purple
		case EItemRarity::Legendary:	return FSlateColor(FLinearColor(1.0f, 0.5f, 0.0f)); // Orange
		case EItemRarity::Common:
		default:						return FSlateColor(FLinearColor::White); 
	}
}

FString UItemTooltipWidget::GetStringForWeaponType(EWeaponType Type) const
{
	switch (Type)
	{
		case EWeaponType::Sword:	return TEXT("Sword");
		case EWeaponType::Axe:		return TEXT("Axe");
		case EWeaponType::Mace:		return TEXT("Mace");
		case EWeaponType::Staff:	return TEXT("Staff");
		case EWeaponType::Dagger:	return TEXT("Dagger");
		case EWeaponType::None:
		default:					return TEXT("Weapon");
	}
}

void UItemTooltipWidget::AddStatRow(const FString& Prefix, float Value, const FSlateColor& Color, const FString& Suffix)
{
	if (!BonusStatsBox || !StatRowClass) return;

	UStatRowWidget* NewRow = CreateWidget<UStatRowWidget>(this, StatRowClass);
	if (NewRow)
	{
		FString StatString = FString::Printf(TEXT("%s%d%s"), *Prefix, FMath::RoundToInt(Value), *Suffix);
		NewRow->InitStatText(FText::FromString(StatString), Color, BonusStatFont);
		
		UVerticalBoxSlot* BoxSlot = BonusStatsBox->AddChildToVerticalBox(NewRow);
		if (BoxSlot)
		{
			BoxSlot->SetPadding(BonusStatPadding);
		}
	}
}
