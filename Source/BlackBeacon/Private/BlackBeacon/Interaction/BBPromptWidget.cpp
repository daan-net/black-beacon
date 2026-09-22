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
	return SAssignNew(PromptTextBlock, STextBlock)
		.Text(CurrentPrompt)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
		.ColorAndOpacity(FLinearColor(1.0f, 0.98f, 0.92f, 1.0f))
		.ShadowOffset(FVector2D(2.0f, 2.0f))
		.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
}

void UBBPromptWidget::SetPromptText(const FText& InText)
{
	CurrentPrompt = InText;

	if (PromptTextBlock.IsValid())
	{
		PromptTextBlock->SetText(CurrentPrompt);
	}

	SetVisibility(CurrentPrompt.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
}
