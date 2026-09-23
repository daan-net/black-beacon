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
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
	
	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(20.0f))
		[
			SAssignNew(ObjectiveTextBlock, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 200.0f, 0.0f, 0.0f)) // below center so it doesn't clutter crosshair
		[
			SAssignNew(PromptTextBlock, STextBlock)
			.Text(CurrentPrompt)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
			.ColorAndOpacity(FLinearColor(1.0f, 0.98f, 0.92f, 1.0f))
			.ShadowOffset(FVector2D(2.0f, 2.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.0f, 50.0f, 0.0f, 0.0f))
		[
			SAssignNew(NotificationTextBlock, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
			.ColorAndOpacity(FLinearColor(0.2f, 1.0f, 0.2f, 1.0f))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		];
}



void UBBPromptWidget::SetPromptText(const FText& InText)
{
	CurrentPrompt = InText;
	if (PromptTextBlock.IsValid())
	{
		PromptTextBlock->SetText(CurrentPrompt);
	}
}

void UBBPromptWidget::SetObjectiveText(const FText& InText)
{
	if (ObjectiveTextBlock.IsValid())
	{
		ObjectiveTextBlock->SetText(InText);
	}
}

void UBBPromptWidget::SetNotificationText(const FText& InText)
{
	if (NotificationTextBlock.IsValid())
	{
		NotificationTextBlock->SetText(InText);
	}
}

