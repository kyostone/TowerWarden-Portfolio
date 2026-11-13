// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMyMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	// 입력(마우스, 키보드) 정보가 UI에만 전달됨
	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);

	if (!IsLocalController()) return;
	if (!MainMenuWidgetClass) return;

	MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);

	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport();
	}
}