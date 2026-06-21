// Fill out your copyright notice in the Description page of Project Settings.


#include "FrameWork/CGameMode.h"
#include "EngineUtils.h"
#include "GameFrameWork/PlayerStart.h"

APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* PlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(PlayerController);
	FGenericTeamId TeamID = GetTeamIDForPlayer(PlayerController);
	if (TeamAgent)
	{
		TeamAgent->SetGenericTeamId(TeamID);
	}

	PlayerController->StartSpot = FindNextStartSpotForTeam(TeamID);
	return PlayerController;
}

FGenericTeamId ACGameMode::GetTeamIDForPlayer(APlayerController* PlayerController) const
{
	static int PlayerCount = 0;
	++PlayerCount;
	return FGenericTeamId(PlayerCount%2);
}

AActor* ACGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const
{
	const FName* StartSpotTag = TeamStartSoptTagMap.Find(TeamID);
	if (!StartSpotTag)
	{
		return nullptr;
	}
	UWorld* World = GetWorld();

	for(TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (PlayerStart->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}
	return nullptr;
}
