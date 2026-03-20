#include "UI/StatRowWidget.h"
#include "Components/TextBlock.h"

void UStatRowWidget::InitStatText(const FText& InStatText, const FSlateColor& InColor, const FSlateFontInfo& InFont)
{
	if (StatText)
	{
		StatText->SetText(InStatText);
		StatText->SetColorAndOpacity(InColor);
		StatText->SetFont(InFont);
	}
}
