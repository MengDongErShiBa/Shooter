// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"

#include "Blueprint/UserWidget.h"

AShooterPlayerController::AShooterPlayerController()
{
	
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 检查HUD覆层Class是否有效
	if (HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
	
}
