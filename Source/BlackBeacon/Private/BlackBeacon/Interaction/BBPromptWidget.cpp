#include "BlackBeacon/Interaction/BBPromptWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Text/STextBlock.h"

void UBBPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetPromptText(CurrentPrompt);
}

TSharedRef<SWidget> UBBPromptWidget::RebuildWidget()
{
	return SNew(STextBlock)
		.Text(CurrentPrompt)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
		.ColorAndOpacity(FLinearColor(1.0f, 0.98f, 0.92f, 1.0f))
		.ShadowOffset(FVector2D(2.0f, 2.0f))
		.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
}

void UBBPromptWidget::SetPromptText(const FText& InText)
{
	CurrentPrompt = InText;

	if (TSharedPtr<SWidget> Cached = GetCachedWidget())
	{
		// Force the slate text to re-read the attribute.
		if (TSharedPtr<STextBlock> TextBlock = StaticCastSharedPtr<STextBlock>(Cached))
		{
			TextBlock->SetText(CurrentPrompt);
		}
	}

	SetVisibility(CurrentPrompt.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
}
