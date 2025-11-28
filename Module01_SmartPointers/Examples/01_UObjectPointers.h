// Example 1: UObject Pointers and Garbage Collection
// This demonstrates proper use of UObject pointers with Unreal's GC system

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "01_UObjectPointers.generated.h"

// Simple UObject-derived data class
UCLASS()
class UPlayerData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int32 Score;

	void PrintData() const
	{
		UE_LOG(LogTemp, Log, TEXT("Player: %s, Score: %d"), *PlayerName, Score);
	}
};

// Example of a manager class that owns UObjects
UCLASS()
class UPlayerManager : public UObject
{
	GENERATED_BODY()

public:
	// CORRECT: UPROPERTY prevents garbage collection
	UPROPERTY()
	UPlayerData* CurrentPlayer;

	// CORRECT: Array of UObject pointers with UPROPERTY
	UPROPERTY()
	TArray<UPlayerData*> AllPlayers;

	// CORRECT: Using TObjectPtr (UE 5.1+) - preferred modern approach
	UPROPERTY()
	TObjectPtr<UPlayerData> ModernPlayerReference;

	void CreatePlayer(const FString& Name, int32 InitialScore)
	{
		// NewObject creates a UObject with this as the outer (owner)
		UPlayerData* NewPlayer = NewObject<UPlayerData>(this);
		NewPlayer->PlayerName = Name;
		NewPlayer->Score = InitialScore;

		// Safe to store - UPROPERTY protects it
		CurrentPlayer = NewPlayer;
		AllPlayers.Add(NewPlayer);
	}

	void DemonstrateWeakReference()
	{
		// For optional references that don't prevent GC
		TWeakObjectPtr<UPlayerData> WeakPlayer = CurrentPlayer;

		// Later, check if still valid
		if (WeakPlayer.IsValid())
		{
			WeakPlayer->PrintData();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player was garbage collected"));
		}
	}
};

// DANGER EXAMPLE: What NOT to do
UCLASS()
class UBadManager : public UObject
{
	GENERATED_BODY()

public:
	// WRONG: No UPROPERTY - this pointer can become dangling!
	// The object can be garbage collected while this pointer still exists
	UPlayerData* UnprotectedPlayer;

	void DangerousPattern()
	{
		// This object might be GC'd if nothing else references it
		UnprotectedPlayer = NewObject<UPlayerData>(this);

		// Later in code... this might crash!
		// UnprotectedPlayer->PrintData();  // DANGER: Might be null or invalid
	}
};

// Example: Actor using UObject components
UCLASS()
class ADataManager : public AActor
{
	GENERATED_BODY()

public:
	// CORRECT: Components are automatically protected
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UPlayerManager* PlayerManager;

	ADataManager()
	{
		// CreateDefaultSubobject is special - used in constructors
		PlayerManager = CreateDefaultSubobject<UPlayerManager>(TEXT("PlayerManager"));
	}

	void BeginPlay() override
	{
		Super::BeginPlay();

		// Using the manager
		PlayerManager->CreatePlayer(TEXT("Alice"), 100);
		PlayerManager->CreatePlayer(TEXT("Bob"), 200);
	}
};
