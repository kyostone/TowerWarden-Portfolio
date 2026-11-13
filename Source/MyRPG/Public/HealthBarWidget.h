// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;

UCLASS()
class MYRPG_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateHealthBar(float NewPercent); 

protected:
	virtual void NativeConstruct() override; // 위젯 생성 시 실행
	virtual void NativeDestruct() override; // 위젯 소멸 시 실행 

	void UpdateHealthBarStep();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(EditDefaultsOnly, Category = "HealthBar")
	float UpdateInterval = 0.03f; // HP바 갱신 주기

	UPROPERTY(EditDefaultsOnly, Category = "HealthBar")
	int32 StepCount = 10; // 목표 HP까지 나눠서 갱신할 횟수

private:
	float CurrentPercent = 1.f; // 출력되는 HP 비율
	float TargetPercent = 1.f;  // 실제 HP 비율
	float StepAmount = 0.f;     // 반복 마다 증가/감소 될 HP

	int32 CurrentStep = 0;		// 현재까지의 반복 횟수

	bool bInitialized = false;	// 최초 HP 값 설정 여부

	FTimerHandle HealthUpdateTimerHandle;
};

