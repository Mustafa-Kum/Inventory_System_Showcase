#include "UI/ItemTooltipWidget.h"
#include "DataAssets/ItemDataAsset.h"
#include "DataAssets/WeaponDataAsset.h"
#include "UI/StatRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Font.h"
#include "GameplayEffect.h"
#include "AttributeSet.h"
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

void UItemTooltipWidget::SetupTooltip(UItemDataAsset* ItemData)
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

void UItemTooltipWidget::UpdateItemName(UItemDataAsset* ItemData)
{
	if (!ItemNameText) return;

	ItemNameText->SetText(ItemData->ItemData.ItemName);
	ItemNameText->SetColorAndOpacity(GetColorForRarity(ItemData->ItemData.Rarity));
}

void UItemTooltipWidget::UpdateItemType(UItemDataAsset* ItemData)
{
	if (!ItemTypeText) return;

	FString TypeStr;
	
	if (UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(ItemData))
	{
		TypeStr = GetStringForWeaponType(WeaponData->WeaponData.WeaponType);
	}
	else
	{
		TypeStr = GetStringForItemType(ItemData->ItemData.ItemType);
	}

	ItemTypeText->SetText(FText::FromString(TypeStr));
}

void UItemTooltipWidget::UpdateBaseStats(UItemDataAsset* ItemData)
{
	if (UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(ItemData))
	{
		if (BaseDamageText)
		{
			BaseDamageText->SetText(FText::Format(NSLOCTEXT("Tooltip", "DamageFmt", "{0} Damage"), FText::AsNumber(FMath::RoundToInt(WeaponData->WeaponData.BaseDamage))));
			BaseDamageText->SetVisibility(ESlateVisibility::Visible);
		}

		if (AttackSpeedText)
		{
			AttackSpeedText->SetText(FText::Format(NSLOCTEXT("Tooltip", "SpeedFmt", "Speed {0}"), FText::AsNumber(WeaponData->WeaponData.WeaponCastSpeed)));
			AttackSpeedText->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		if (BaseDamageText) BaseDamageText->SetVisibility(ESlateVisibility::Collapsed);
		if (AttackSpeedText) AttackSpeedText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UItemTooltipWidget::UpdateBonusStats(UItemDataAsset* ItemData)
{
	if (!BonusStatsBox || !StatRowClass) return;

	BonusStatsBox->ClearChildren();
	
	if (!ItemData->ItemData.EquippedStatEffect) return;

	// AAA CDO Parsing: Automatically generate tooltip from the Gameplay Effect's modifiers
	if (UGameplayEffect* GE = ItemData->ItemData.EquippedStatEffect.GetDefaultObject())
	{
		const FSlateColor GreenColor = FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f));
		const FSlateColor RedColor = FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f));

		for (const auto& ModInfo : GE->Modifiers)
		{
			float Magnitude = 0.0f;
			if (ModInfo.ModifierMagnitude.GetStaticMagnitudeIfPossible(1.0f, Magnitude))
			{
				if (Magnitude == 0.0f) continue;
				
				FString AttributeName = ModInfo.Attribute.GetName();
				FString Prefix = Magnitude > 0.0f ? TEXT("+") : TEXT("");
				FSlateColor Color = Magnitude > 0.0f ? GreenColor : RedColor;
				
				AddStatRow(Prefix, Magnitude, Color, FString::Printf(TEXT(" %s"), *AttributeName));
			}
		}
	}
}

void UItemTooltipWidget::UpdateEconomy(UItemDataAsset* ItemData)
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

FString UItemTooltipWidget::GetStringForItemType(EItemType Type) const
{
	switch (Type)
	{
		case EItemType::Weapon:		return TEXT("Weapon");
		case EItemType::Armor:		return TEXT("Armor");
		case EItemType::Consumable:	return TEXT("Consumable");
		case EItemType::Quest:		return TEXT("Quest Item");
		case EItemType::None:
		default:					return TEXT("Item");
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
