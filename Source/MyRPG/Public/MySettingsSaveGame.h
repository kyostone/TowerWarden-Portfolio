#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MySettingsSaveGame.generated.h"

UCLASS()
class MYRPG_API UMySettingsSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, SaveGame)
    float MasterVolume = 1.f;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    float BGMVolume = 1.f;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    float SFXVolume = 1.f;
};