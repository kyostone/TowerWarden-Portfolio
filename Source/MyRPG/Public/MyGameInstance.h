// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 *
 */

class UMySaveGame;
class UStageDataAsset;
class UWorld;
class UMySettingsSaveGame;
class USoundClass;
class USoundMix;

UENUM(BlueprintType)
enum class EStageStartMode : uint8
{
	Solo,
	Host
};

// 스테이지 정보
USTRUCT(BlueprintType)
struct FStageRuntimeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FPrimaryAssetId StageId;

	UPROPERTY(BlueprintReadOnly)
	int32 StageIndex = 1;

	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UWorld> StageMap;

	UPROPERTY(BlueprintReadOnly)
	int32 StartWave = 1;

	UPROPERTY(BlueprintReadOnly)
	FPrimaryAssetId NextStageId;
};

UCLASS()
class MYRPG_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	virtual void Init() override;

	void SaveGame();
	void LoadGame();

	virtual void OnStart() override;

	// 저장된 오디오 설정을 실제 사운드에 적용
	void ApplyAudioSettings();

	UFUNCTION(BlueprintCallable)
	void ReturnToMainMenu();
	bool CanModifyProgressSave() const;//세이브 가능 여부(싱글 플레이, 호스트)
	void StartGameSession();
	// Stage
	UFUNCTION(BlueprintCallable)
	void ClearStage(int32 StageNumber);

	UFUNCTION(BlueprintPure)
	bool IsStageCleared(int32 StageNumber) const;

	UFUNCTION(BlueprintPure)
	int32 GetHighestUnlockedStage() const;


	// Continue
	UFUNCTION(BlueprintCallable)
	void SaveContinueData(int32 StageNumber, int32 WaveNumber);

	UFUNCTION(BlueprintCallable)
	void ClearContinueData(int32 StageNumber);

	UFUNCTION(BlueprintPure)
	bool HasContinueData() const;

	UFUNCTION(BlueprintPure)
	int32 GetContinueWave(int32 StageNumber) const;

	UFUNCTION(BlueprintCallable, Category = "Stage")
	void StartStage(FPrimaryAssetId StageId, bool bContinue, bool bHostGame);

	UFUNCTION(BlueprintCallable, Category = "Stage")
	void StartNextStage();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveCurrentProgress();

	// Settings
	void LoadSettings();

	UFUNCTION(BlueprintCallable)
	void SaveSettings();

	UFUNCTION(BlueprintCallable)
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable)
	void SetBGMVolume(float Volume);

	UFUNCTION(BlueprintCallable)
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintPure)
	float GetMasterVolume() const;

	UFUNCTION(BlueprintPure)
	float GetBGMVolume() const;

	UFUNCTION(BlueprintPure)
	float GetSFXVolume() const;

	// Audio
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Settings")
	TObjectPtr<USoundMix> SettingsSoundMix = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Settings")
	TObjectPtr<USoundClass> MasterSoundClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Settings")
	TObjectPtr<USoundClass> BGMSoundClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Settings")
	TObjectPtr<USoundClass> SFXSoundClass = nullptr;

	//Session
	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostGame(const FString& MapName, int32 StartWave);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindGame();

	UFUNCTION(BlueprintPure, Category = "Stage")
	UStageDataAsset* GetStageData(FPrimaryAssetId StageId) const;

	UFUNCTION(BlueprintPure, Category = "Stage")
	FStageRuntimeInfo GetStageRuntimeInfo(FPrimaryAssetId StageId, bool bContinue) const;

	UFUNCTION(BlueprintPure, Category = "Stage")
	const FStageRuntimeInfo& GetCurrentStageInfo() const { return CurrentStageInfo; }

private:
	UPROPERTY()
	TObjectPtr<UMySaveGame> SaveData;
	const FString SaveSlotName = TEXT("MyRPG_Save");

	UPROPERTY()
	TObjectPtr<UMySettingsSaveGame> SettingsData;
	const FString SettingsSlotName = TEXT("MyRPG_Settings");

	FStageRuntimeInfo CurrentStageInfo;

	bool bCurrentHostGame = false;

	//Session
	// 세션 생성 / 검색 / 참가 인터페이스
	IOnlineSessionPtr SessionInterface;

	// 검색 결과를 비동기로 받을 때까지 유지
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// 세션 생성 완료 후 이동할 맵
	FString PendingHostMapName;
	// 세션 생성 완료 후 진행할 웨이브
	int32 PendingHostStartWave = 1;	

	void OnCreateSessionComplete(FName SessionName,bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName,EOnJoinSessionCompleteResult::Type Result);	
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
};
