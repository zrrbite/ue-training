// Exercise 1: Basic Smart Pointer Usage
// Fix the code and implement the missing parts

#pragma once

#include "CoreMinimal.h"

/*
 * EXERCISE 1: Fix UObject Pointer Management
 *
 * The code below has memory management issues.
 * Fix all the issues marked with TODO.
 */

UCLASS()
class UDataStore : public UObject
{
	GENERATED_BODY()

public:
	FString StoredData;
};

UCLASS()
class UDataManager : public UObject
{
	GENERATED_BODY()

public:
	// TODO: Fix this - it needs UPROPERTY to prevent garbage collection
	UDataStore* PrimaryStore;

	// TODO: Fix this - should use TWeakObjectPtr for optional reference
	UDataStore* OptionalStore;

	void Initialize()
	{
		PrimaryStore = NewObject<UDataStore>(this);
		PrimaryStore->StoredData = TEXT("Important Data");

		OptionalStore = NewObject<UDataStore>(this);
		OptionalStore->StoredData = TEXT("Temporary Data");
	}

	void UseStores()
	{
		// TODO: Add null check for PrimaryStore

		// TODO: Add proper weak pointer check for OptionalStore
	}
};

/*
 * EXERCISE 2: Choose the Right Pointer Type
 *
 * For each scenario, choose the correct pointer type and implement it.
 */

// Scenario A: Multiple systems need to share player stats
struct FPlayerStats
{
	int32 Health;
	int32 Mana;
};

class FScenarioA
{
public:
	// TODO: Choose correct pointer type for shared stats
	// ??? PlayerStats;

	void Initialize()
	{
		// TODO: Create the shared stats
	}
};

// Scenario B: A file handle that should only have one owner
class FFileHandle
{
public:
	FString FileName;
	~FFileHandle()
	{
		UE_LOG(LogTemp, Log, TEXT("Closing file: %s"), *FileName);
	}
};

class FScenarioB
{
public:
	// TODO: Choose correct pointer type for exclusive ownership
	// ??? FileHandle;

	void OpenFile(const FString& Name)
	{
		// TODO: Create the file handle with unique ownership
	}
};

// Scenario C: Observer pattern - observers shouldn't keep subject alive
class FEventDispatcher
{
public:
	// TODO: Choose correct pointer type for observers
	// TArray<???> Observers;

	void NotifyAll()
	{
		// TODO: Implement notification with proper weak pointer handling
	}
};

/*
 * EXERCISE 3: Fix the Memory Leak
 *
 * The code below creates a circular reference that will leak memory.
 * Fix it using the appropriate weak pointer.
 */

class FNode
{
public:
	FString NodeName;

	// Strong reference to next node
	TSharedPtr<FNode> Next;

	// TODO: Fix this - should be weak to break the cycle
	TSharedPtr<FNode> Previous;

	explicit FNode(const FString& Name)
		: NodeName(Name)
	{
	}
};

class FLinkedList
{
public:
	TSharedPtr<FNode> Head;

	void CreateCircularReference()
	{
		// This creates a cycle: Head -> Node2 -> Head
		Head = MakeShared<FNode>(TEXT("Node1"));
		TSharedPtr<FNode> Node2 = MakeShared<FNode>(TEXT("Node2"));

		Head->Next = Node2;
		Node2->Previous = Head;  // TODO: This should use weak pointer

		// Memory leak! Neither node can be deleted.
		// Fix the FNode class above.
	}
};

// See Exercise01_BasicPointers_Solution.h for answers
