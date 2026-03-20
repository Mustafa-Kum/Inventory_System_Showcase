// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "StatRowWidget.generated.h"

/**
 * AAA: Reusable widget for representing a single row of a bonus stat (e.g., "+15 Versatility").
 */
UCLASS(Abstract)
class WOWCLONE_API UStatRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set the stat text
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void InitStatText(const FText& InStatText, const FSlateColor& InColor, const FSlateFontInfo& InFont);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> StatText;
};
