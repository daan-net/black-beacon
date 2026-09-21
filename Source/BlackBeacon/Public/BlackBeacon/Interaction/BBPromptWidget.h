// BLACK BEACON - interaction prompt widget.
//
// A code-only UUserWidget (Slate-backed) that shows the focused
// interactable's prompt. No UMG assets required - keeps the project
// fully text/C++ until the UI pass in 0.2.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "BBPromptWidget.generated.h"

class STextBlock;

UCLASS()
class UBBPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Sets the prompt text; empty text hides the widget.
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|UI")
	void SetPromptText(const FText& InText);

	const FText& GetCurrentPrompt() const { return CurrentPrompt; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	// Slate attribute source used inside RebuildWidget.
	const FText& GetPromptTextSrc() const { return CurrentPrompt; }

	UPROPERTY()
	FText CurrentPrompt;
};