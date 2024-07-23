// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenuWidget.h"
#include "Kismet/GameplayStatics.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeButtonClicked);
	ResumeButton->OnHovered.AddDynamic(this, &UPauseMenuWidget::OnResumeButtonHovered);
	ResumeButton->OnUnhovered.AddDynamic(this, &UPauseMenuWidget::OnResumeButtonUnhovered);

	class APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (playerController)
	{
		playerController->SetInputMode(FInputModeUIOnly());
		playerController->SetShowMouseCursor(true);
		UGameplayStatics::SetGamePaused(this,true);
	}
}


void UPauseMenuWidget::OnResumeButtonHovered()
{
	if (!OnHover.IsNull())
	{
		PlayAnimation(OnHover.Get());
	}
}

void UPauseMenuWidget::OnResumeButtonUnhovered()
{
	if (!OnHover.IsNull())
	{
		PlayAnimation(OnHover.Get(),0,1,EUMGSequencePlayMode::Reverse);
	}
}

void UPauseMenuWidget::OnResumeButtonClicked()
{
	class APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (playerController)
	{
		playerController->SetInputMode(FInputModeGameOnly());
		playerController->SetShowMouseCursor(false);
		UGameplayStatics::SetGamePaused(this,false);
		RemoveFromViewport();
	}
}
