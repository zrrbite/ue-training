// Exercise 1: Solutions

#pragma once

#include "CoreMinimal.h"

/*
 * SOLUTION 1: Fixed UObject Pointer Management
 */

UCLASS()
class UDataStore_Solution : public UObject
{
	GENERATED_BODY()

public:
	FString StoredData;
};

UCLASS()
class UDataManager_Solution : public UObject
{
	GENERATED_BODY()

public:
	// FIXED: Added UPROPERTY to prevent garbage collection
	UPROPERTY()
	UDataStore_Solution* PrimaryStore;

	// FIXED: Using TWeakObjectPtr for optional reference
	TWeakObjectPtr<UDataStore_Solution> OptionalStore;

	void Initialize()
	{
		PrimaryStore = NewObject<UDataStore_Solution>(this);
		PrimaryStore->StoredData = TEXT("Important Data");

		UDataStore_Solution* TempStore = NewObject<UDataStore_Solution>(this);
		TempStore->StoredData = TEXT("Temporary Data");
		OptionalStore = TempStore;
	}

	void UseStores()
	{
		// FIXED: Added null check
		if (PrimaryStore)
		{
			UE_LOG(LogTemp, Log, TEXT("Primary: %s"), *PrimaryStore->StoredData);
		}

		// FIXED: Proper weak pointer validation
		if (OptionalStore.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("Optional: %s"), *OptionalStore->StoredData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Optional store was garbage collected"));
		}
	}
};

/*
 * SOLUTION 2: Correct Pointer Types for Each Scenario
 */

// Scenario A: TSharedPtr for shared ownership
struct FPlayerStats_Solution
{
	int32 Health;
	int32 Mana;
};

class FScenarioA_Solution
{
public:
	// SOLUTION: TSharedPtr for shared ownership
	TSharedPtr<FPlayerStats_Solution> PlayerStats;

	void Initialize()
	{
		// Create shared stats that can be passed to multiple systems
		PlayerStats = MakeShared<FPlayerStats_Solution>();
		PlayerStats->Health = 100;
		PlayerStats->Mana = 50;
	}

	void ShareWithOtherSystem()
	{
		// Other systems can hold TSharedPtr to same stats
		TSharedPtr<FPlayerStats_Solution> SharedStats = PlayerStats;
		// Both pointers keep the stats alive
	}
};

// Scenario B: TUniquePtr for exclusive ownership
class FFileHandle_Solution
{
public:
	FString FileName;

	explicit FFileHandle_Solution(const FString& Name)
		: FileName(Name)
	{
		UE_LOG(LogTemp, Log, TEXT("Opening file: %s"), *FileName);
	}

	~FFileHandle_Solution()
	{
		UE_LOG(LogTemp, Log, TEXT("Closing file: %s"), *FileName);
	}
};

class FScenarioB_Solution
{
public:
	// SOLUTION: TUniquePtr for exclusive ownership
	TUniquePtr<FFileHandle_Solution> FileHandle;

	void OpenFile(const FString& Name)
	{
		// Create with unique ownership - only this class owns it
		FileHandle = MakeUnique<FFileHandle_Solution>(Name);
	}

	void TransferOwnership()
	{
		// Can move ownership to another unique_ptr
		TUniquePtr<FFileHandle_Solution> NewOwner = MoveTemp(FileHandle);
		// FileHandle is now null, NewOwner owns the file
	}
};

// Scenario C: TWeakPtr for observers
class IObserver_Solution
{
public:
	virtual ~IObserver_Solution() = default;
	virtual void OnEvent() = 0;
};

class FConcreteObserver_Solution : public IObserver_Solution
{
public:
	FString Name;

	explicit FConcreteObserver_Solution(const FString& InName)
		: Name(InName)
	{
	}

	void OnEvent() override
	{
		UE_LOG(LogTemp, Log, TEXT("Observer %s notified"), *Name);
	}
};

class FEventDispatcher_Solution
{
public:
	// SOLUTION: TWeakPtr so observers don't keep dispatcher alive
	TArray<TWeakPtr<IObserver_Solution>> Observers;

	void AddObserver(TSharedPtr<IObserver_Solution> Observer)
	{
		Observers.Add(Observer);
	}

	void NotifyAll()
	{
		// Clean up dead observers
		Observers.RemoveAll([](const TWeakPtr<IObserver_Solution>& Weak)
		{
			return !Weak.IsValid();
		});

		// Notify living observers
		for (TWeakPtr<IObserver_Solution>& WeakObserver : Observers)
		{
			if (TSharedPtr<IObserver_Solution> Observer = WeakObserver.Pin())
			{
				Observer->OnEvent();
			}
		}
	}
};

/*
 * SOLUTION 3: Fixed Circular Reference
 */

class FNode_Solution
{
public:
	FString NodeName;

	// Strong reference forward
	TSharedPtr<FNode_Solution> Next;

	// FIXED: Weak reference backward to break cycle
	TWeakPtr<FNode_Solution> Previous;

	explicit FNode_Solution(const FString& Name)
		: NodeName(Name)
	{
		UE_LOG(LogTemp, Log, TEXT("Node created: %s"), *NodeName);
	}

	~FNode_Solution()
	{
		UE_LOG(LogTemp, Log, TEXT("Node destroyed: %s"), *NodeName);
	}
};

class FLinkedList_Solution
{
public:
	TSharedPtr<FNode_Solution> Head;

	void CreateProperStructure()
	{
		// No longer creates a cycle
		Head = MakeShared<FNode_Solution>(TEXT("Node1"));
		TSharedPtr<FNode_Solution> Node2 = MakeShared<FNode_Solution>(TEXT("Node2"));

		Head->Next = Node2;
		Node2->Previous = Head;  // Weak reference - breaks the cycle

		// When these go out of scope, both nodes are properly deleted
	}

	void Demonstrate()
	{
		CreateProperStructure();
		// Nodes are deleted here - no memory leak!
	}
};

// Test function to verify the solution
inline void TestSolutions()
{
	UE_LOG(LogTemp, Log, TEXT("=== Testing Circular Reference Fix ==="));
	{
		FLinkedList_Solution List;
		List.Demonstrate();
		// Watch the log - both nodes should be destroyed
	}

	UE_LOG(LogTemp, Log, TEXT("=== Testing Observer Pattern ==="));
	{
		FEventDispatcher_Solution Dispatcher;

		{
			TSharedPtr<IObserver_Solution> Observer1 = MakeShared<FConcreteObserver_Solution>(TEXT("Obs1"));
			TSharedPtr<IObserver_Solution> Observer2 = MakeShared<FConcreteObserver_Solution>(TEXT("Obs2"));

			Dispatcher.AddObserver(Observer1);
			Dispatcher.AddObserver(Observer2);

			Dispatcher.NotifyAll();  // Both notified
		}
		// Observers destroyed

		Dispatcher.NotifyAll();  // No observers - handled safely
	}
}
