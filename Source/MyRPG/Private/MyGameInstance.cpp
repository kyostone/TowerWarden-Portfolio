// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "MySaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerController.h"
#include "StageDataAsset.h"
#include "Engine/AssetManager.h"
#include "MySettingsSaveGame.h"
#include "MyRPGGameState.h"

namespace
{
	const FName MyRPGSessionName(TEXT("MyRPGSession"));
}

void UMyGameInstance::Init()
{
	Super::Init();

	LoadGame();
	LoadSettings();

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem not found"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("OnlineSubsystem initialized: %s"), *OnlineSubsystem->GetSubsystemName().ToString());

	SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SessionInterface initialized"));

	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnCreateSessionComplete));
	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnFindSessionsComplete));
	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnJoinSessionComplete));
	SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this,&UMyGameInstance::OnDestroySessionComplete));
}

void UMyGameInstance::OnStart()
{
	Super::OnStart();

	if (!SettingsSoundMix)
	{
		UE_LOG(LogTemp, Error, TEXT("[Audio] SettingsSoundMix is NULL!"));
		return;
	}

	// Sound Mix 활성화
	UGameplayStatics::PushSoundMixModifier(this,SettingsSoundMix.Get());

	ApplyAudioSettings();
}

void UMyGameInstance::LoadGame()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		SaveData = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	}

	if (!SaveData)
	{
		SaveData = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));
		SaveGame();
	}
}

void UMyGameInstance::LoadSettings()
{
	UE_LOG(LogTemp, Warning, TEXT("LoadSettings Called!"));

	if (UGameplayStatics::DoesSaveGameExist(SettingsSlotName, 0))
	{
		SettingsData = Cast<UMySettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SettingsSlotName, 0));

		if (SettingsData) return;

		UE_LOG(LogTemp, Error, TEXT("Failed to load settings"));
	}
	SettingsData = Cast<UMySettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UMySettingsSaveGame::StaticClass()));
	if (!SettingsData) return;

	SaveSettings();
}

void UMyGameInstance::ReturnToMainMenu()
{
	SaveCurrentProgress();

	if (!SessionInterface.IsValid())
	{
		UGameplayStatics::OpenLevel(
			this, FName(TEXT("Lvl_MainMenu")));
		return;
	}

	if (!SessionInterface->DestroySession(MyRPGSessionName))
	{
		UGameplayStatics::OpenLevel(
			this, FName(TEXT("Lvl_MainMenu")));
	}
}
// 세션 삭제 성공 시 실행
void UMyGameInstance::OnDestroySessionComplete(FName SessionName,bool bWasSuccessful)
{
	UGameplayStatics::OpenLevel(this,FName(TEXT("Lvl_MainMenu")));
}

void UMyGameInstance::SaveGame()
{
	if (!SaveData) return;
	UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, 0);
}

//세이브 가능 여부(싱글 플레이, 호스트)
bool UMyGameInstance::CanModifyProgressSave() const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	const ENetMode NetMode = World->GetNetMode();

	return NetMode == NM_Standalone || NetMode == NM_ListenServer;
}

// 스테이지 시작 시 접속 차단
void UMyGameInstance::StartGameSession()
{
	if (!bCurrentHostGame || !SessionInterface.IsValid()) return;

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(MyRPGSessionName);
	if (!Session) return;

	Session->SessionSettings.bAllowJoinInProgress = false;
	SessionInterface->StartSession(MyRPGSessionName);
}

// Stage

void UMyGameInstance::ClearStage(int32 StageNumber)
{
	if (!SaveData || StageNumber <= 0 || !CanModifyProgressSave()) return;

	const int32 StageIndex = StageNumber - 1;

	if (SaveData->ContinueWaves.IsValidIndex(StageIndex))
	{
		// 스테이지 클리어 시 기존 체크포인트는 더 이상 필요하지 않으므로 제거
		SaveData->ContinueWaves[StageIndex] = 0;
	}	

	// 다음 스테이지 해금
	SaveData->HighestUnlockedStage = FMath::Max(SaveData->HighestUnlockedStage, StageNumber + 1);

	SaveGame();
}


bool UMyGameInstance::IsStageCleared(int32 StageNumber) const
{
	if (!SaveData || StageNumber <= 0) return false;

	return SaveData->HighestUnlockedStage > StageNumber;
}


int32 UMyGameInstance::GetHighestUnlockedStage() const
{
	if (!SaveData) return 1;

	return SaveData->HighestUnlockedStage;
}

// Continue

void UMyGameInstance::SaveContinueData(int32 StageNumber, int32 WaveNumber)
{
	if (!SaveData || StageNumber <= 0 || WaveNumber <= 0 || !CanModifyProgressSave()) return;

	const int32 StageIndex = StageNumber - 1;

	if (SaveData->ContinueWaves.Num() <= StageIndex)
	{
		SaveData->ContinueWaves.SetNumZeroed(StageIndex + 1);
	}

	SaveData->ContinueWaves[StageIndex] = WaveNumber;

	SaveGame();
}

void UMyGameInstance::ClearContinueData(int32 StageNumber)
{
	if (!SaveData || StageNumber <= 0 || !CanModifyProgressSave()) return;

	const int32 StageIndex = StageNumber - 1;

	if (!SaveData->ContinueWaves.IsValidIndex(StageIndex)) return;

	SaveData->ContinueWaves[StageIndex] = 0;

	SaveGame();
}

// 이어하기 데이터 유무 확인
bool UMyGameInstance::HasContinueData() const
{
	if (!SaveData) return false;

	for (const int32 Wave : SaveData->ContinueWaves)
	{
		if (Wave > 0) return true;
	}

	return false;
}

int32 UMyGameInstance::GetContinueWave(int32 StageNumber) const
{
	if (!SaveData || StageNumber <= 0) return 0;

	const int32 StageIndex = StageNumber - 1;

	if (!SaveData->ContinueWaves.IsValidIndex(StageIndex)) return 0;

	return SaveData->ContinueWaves[StageIndex];
}

void UMyGameInstance::StartStage(FPrimaryAssetId StageId, bool bContinue, bool bHostGame)
{
	FStageRuntimeInfo StageInfo = GetStageRuntimeInfo(StageId, bContinue); // 해당 스테이지 정보

	if (!StageInfo.StageId.IsValid()) return;
	if (StageInfo.StageMap.IsNull()) return;

	CurrentStageInfo = StageInfo;
	bCurrentHostGame = bHostGame;

	// StageMap에서 맵 에셋의 패키지 경로를 가져옴
	const FString MapPath = StageInfo.StageMap.ToSoftObjectPath().GetLongPackageName();
	if (MapPath.IsEmpty()) return;

	//호스트 일때
	if (bHostGame)
	{
		HostGame(MapPath, StageInfo.StartWave);
		return;
	}

	const FString Options = FString::Printf(TEXT("StartWave=%d"), StageInfo.StartWave); // 시작할 스테이지 넘겨줌

	UGameplayStatics::OpenLevel(this, FName(*MapPath), true, Options);
}

// 다음 스테이지 시작
void UMyGameInstance::StartNextStage()
{
	if (!CurrentStageInfo.NextStageId.IsValid()) return;

	// Solo일 경우
	if (!bCurrentHostGame)
	{
		StartStage(CurrentStageInfo.NextStageId, false, false);
		return;
	}

	// Host일 경우
	FStageRuntimeInfo NextStageInfo = GetStageRuntimeInfo(CurrentStageInfo.NextStageId, false); // 다음 스테이지 정보

	if (!NextStageInfo.StageId.IsValid()) return;
	if (NextStageInfo.StageMap.IsNull()) return;

	// 다음 스테이지 맵 경로
	const FString MapPath = NextStageInfo.StageMap.ToSoftObjectPath().GetLongPackageName();
	if (MapPath.IsEmpty()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	CurrentStageInfo = NextStageInfo; // 스테이지 정보 교체

	// 다음 스테이지 정보로 서버 이동 주소 생성
	const FString TravelURL = FString::Printf(TEXT("%s?listen?StartWave=%d"), *MapPath, NextStageInfo.StartWave);

	// 호스트와 클라이언트 다음 스테이지 이동
	World->ServerTravel(TravelURL);
}

UStageDataAsset* UMyGameInstance::GetStageData(FPrimaryAssetId StageId) const
{
	if (!StageId.IsValid()) return nullptr;

	UAssetManager& AssetManager = UAssetManager::Get();

	// StageId에 해당하는 Primary Asset의 경로를 가져옴
	const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(StageId);
	if (!AssetPath.IsValid()) return nullptr;

	return Cast<UStageDataAsset>(AssetPath.TryLoad());
}

//게임 진행에 필요한 정보 받기
FStageRuntimeInfo UMyGameInstance::GetStageRuntimeInfo(FPrimaryAssetId StageId, bool bContinue) const
{
	FStageRuntimeInfo Info;

	UStageDataAsset* StageData = GetStageData(StageId);
	if (!StageData) return Info;

	Info.StageId = StageId;
	Info.StageIndex = StageData->StageIndex;
	Info.StageMap = StageData->StageMap;
	Info.StartWave = StageData->StartWave;
	Info.NextStageId = StageData->NextStageId;

	// 저장정보 있을 시
	if (bContinue && SaveData)
	{
		const int32 SaveIndex = StageData->StageIndex - 1;

		if (SaveData->ContinueWaves.IsValidIndex(SaveIndex))
		{
			const int32 SavedWave = SaveData->ContinueWaves[SaveIndex];

			if (SavedWave > 0)
			{
				Info.StartWave = SavedWave;
			}
		}
	}

	return Info;
}

void UMyGameInstance::SaveCurrentProgress()
{
	if (!SaveData || !CanModifyProgressSave()) return;
	if (!CurrentStageInfo.StageId.IsValid()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AMyRPGGameState* GS = World->GetGameState<AMyRPGGameState>();
	if (!GS) return;

	const FWaveUIState& WaveState = GS->GetWaveUIState();

	const int32 StageNumber = CurrentStageInfo.StageIndex;

	// 스테이지 클리어 상태
	if (WaveState.Phase == EWavePhase::GameClear)
	{
		ClearStage(StageNumber);
		return;
	}

	int32 WaveNumber = CurrentStageInfo.StartWave;

	switch (WaveState.Phase)
	{
	case EWavePhase::Active:
	case EWavePhase::Countdown:
		WaveNumber = WaveState.CurrentWave;
		break;

	case EWavePhase::Cleared:
		WaveNumber = WaveState.CurrentWave + 1;
		break;

	case EWavePhase::Waiting:
		break;

	default:
		return;
	}

	if (WaveState.TotalWave > 0)
	{
		WaveNumber = FMath::Clamp(WaveNumber, 1, WaveState.TotalWave);
	}

	SaveContinueData(StageNumber, WaveNumber);
}

// Settings

void UMyGameInstance::SetMasterVolume(float Volume)
{
	if (!SettingsData) return;

	SettingsData->MasterVolume = FMath::Clamp(Volume, 0.f, 1.f);

	ApplyAudioSettings();
}

void UMyGameInstance::SetBGMVolume(float Volume)
{
	if (!SettingsData) return;

	SettingsData->BGMVolume = FMath::Clamp(Volume, 0.f, 1.f);

	ApplyAudioSettings();
}

void UMyGameInstance::SetSFXVolume(float Volume)
{
	if (!SettingsData) return;

	SettingsData->SFXVolume = FMath::Clamp(Volume, 0.f, 1.f);

	ApplyAudioSettings();
}

void UMyGameInstance::SaveSettings()
{
	if (!SettingsData) return;

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SettingsData,SettingsSlotName,0);

	if (!bSaved)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save settings"));
	}
}

void UMyGameInstance::ApplyAudioSettings()
{
	if (!SettingsData || !GetWorld()) return;

	if (!SettingsSoundMix ||
		!MasterSoundClass ||
		!BGMSoundClass ||
		!SFXSoundClass)
	{
		return;
	}

	const float MasterVolume = FMath::Clamp(SettingsData->MasterVolume, 0.f, 1.f);

	const float BGMVolume = MasterVolume * FMath::Clamp(SettingsData->BGMVolume, 0.f, 1.f);

	const float SFXVolume =	MasterVolume * FMath::Clamp(SettingsData->SFXVolume, 0.f, 1.f);

	constexpr float FadeTime = 0.05f;

	// 전체 볼륨: Master에 직접 할당된 사운드
	UGameplayStatics::SetSoundMixClassOverride(this,SettingsSoundMix.Get(),MasterSoundClass.Get(),MasterVolume,1.f,FadeTime,false);

	// BGM: 전체 볼륨과 BGM 볼륨 적용
	UGameplayStatics::SetSoundMixClassOverride(this,SettingsSoundMix.Get(),BGMSoundClass.Get(),BGMVolume,1.f,FadeTime,false);

	// SFX: 전체 볼륨과 효과음 볼륨 적용
	UGameplayStatics::SetSoundMixClassOverride(this,SettingsSoundMix.Get(),SFXSoundClass.Get(),SFXVolume,1.f,FadeTime,false);
}

float UMyGameInstance::GetMasterVolume() const
{
	if (!SettingsData) return 1.f;

	return SettingsData->MasterVolume;
}


float UMyGameInstance::GetBGMVolume() const
{
	if (!SettingsData) return 1.f;

	return SettingsData->BGMVolume;
}


float UMyGameInstance::GetSFXVolume() const
{
	if (!SettingsData) return 1.f;

	return SettingsData->SFXVolume;
}

void UMyGameInstance::HostGame(const FString& MapName, int32 StartWave)
{
	UE_LOG(LogTemp, Warning, TEXT("HOST GAME CALLED"));

	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HostGame failed: invalid SessionInterface"));
		return;
	}

	PendingHostMapName = MapName;
	PendingHostStartWave = StartWave;

	FOnlineSessionSettings SessionSettings;

	// NULL Subsystem이므로 LAN 세션
	SessionSettings.bIsLANMatch = true;

	// 다른 플레이어에게 검색되도록 설정
	SessionSettings.bShouldAdvertise = true;

	// 호스트 포함 최대 2명
	SessionSettings.NumPublicConnections = 2;

	// 참가 허용
	SessionSettings.bAllowJoinInProgress = true;

	const bool bCreateStarted = SessionInterface->CreateSession(0, MyRPGSessionName, SessionSettings);

	if (!bCreateStarted)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSession failed to start"));
	}
}

void UMyGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("Session creation failed"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Session created successfully"));

	if (PendingHostMapName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Host map name is empty"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Opening listen server map: %s"), *PendingHostMapName);

	const FString Options = FString::Printf(TEXT("listen?StartWave=%d"), PendingHostStartWave);
	UGameplayStatics::OpenLevel(this, FName(*PendingHostMapName), true, Options);
}

// 참가 가능한 세션 검색
void UMyGameInstance::FindGame()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("FindAndJoinGame failed: invalid SessionInterface"));
		return;
	}

	SessionSearch = MakeShared<FOnlineSessionSearch>();

	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 10;

	UE_LOG(LogTemp, Log, TEXT("Searching for sessions"));

	const bool bSearchStarted = SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());

	if (!bSearchStarted)
	{
		UE_LOG(LogTemp, Error, TEXT("FindSessions failed to start"));
	}
}

//세션 검색 완료 시
void UMyGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful || !SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Session search failed"));
		return;
	}

	const int32 ResultCount = SessionSearch->SearchResults.Num();

	UE_LOG(LogTemp, Log, TEXT("Found sessions: %d"), ResultCount);

	if (ResultCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No available sessions found"));
		return;
	}

	const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[0];

	UE_LOG(LogTemp, Log, TEXT("Joining first session"));

	//세션 참가
	SessionInterface->JoinSession(0, MyRPGSessionName, Result);
}

//세션 참가 완료 시
void UMyGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("JoinSession failed"));
		return;
	}

	FString ConnectAddress;

	if (!SessionInterface->GetResolvedConnectString(SessionName, ConnectAddress))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to resolve connect address"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Resolved server address: %s"), *ConnectAddress);

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) return;

	UE_LOG(LogTemp, Log, TEXT("Client travel started"));

	PlayerController->ClientTravel(ConnectAddress, TRAVEL_Absolute);
}
