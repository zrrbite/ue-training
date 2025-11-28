// Example 2: TSharedPtr - Shared Ownership for Non-UObjects
// Use for plain C++ classes that need shared ownership

#pragma once

#include "CoreMinimal.h"

// Plain C++ struct - NOT a UObject, so we can use TSharedPtr
struct FPlayerStats
{
	FString PlayerName;
	int32 Health;
	int32 Mana;
	TArray<FString> Inventory;

	FPlayerStats(const FString& Name)
		: PlayerName(Name)
		, Health(100)
		, Mana(50)
	{
	}

	void PrintStats() const
	{
		UE_LOG(LogTemp, Log, TEXT("Player: %s | HP: %d | MP: %d | Items: %d"),
			*PlayerName, Health, Mana, Inventory.Num());
	}
};

// Example: Multiple systems sharing data
class FStatsDisplay
{
public:
	TSharedPtr<FPlayerStats> Stats;

	explicit FStatsDisplay(TSharedPtr<FPlayerStats> InStats)
		: Stats(InStats)
	{
	}

	void Update()
	{
		if (Stats.IsValid())
		{
			Stats->PrintStats();
		}
	}
};

class FStatsModifier
{
public:
	TSharedPtr<FPlayerStats> Stats;

	explicit FStatsModifier(TSharedPtr<FPlayerStats> InStats)
		: Stats(InStats)
	{
	}

	void ApplyDamage(int32 Damage)
	{
		if (Stats.IsValid())
		{
			Stats->Health -= Damage;
			UE_LOG(LogTemp, Log, TEXT("%s took %d damage!"), *Stats->PlayerName, Damage);
		}
	}
};

// Example usage class
class FSharedPtrExample
{
public:
	void RunExample()
	{
		// Create shared pointer - reference count = 1
		TSharedPtr<FPlayerStats> PlayerStats = MakeShared<FPlayerStats>(TEXT("Hero"));

		UE_LOG(LogTemp, Log, TEXT("Reference count: %d"), PlayerStats.GetSharedReferenceCount());
		// Output: Reference count: 1

		// Share with display system - reference count = 2
		FStatsDisplay Display(PlayerStats);

		UE_LOG(LogTemp, Log, TEXT("Reference count: %d"), PlayerStats.GetSharedReferenceCount());
		// Output: Reference count: 2

		// Share with modifier system - reference count = 3
		FStatsModifier Modifier(PlayerStats);

		UE_LOG(LogTemp, Log, TEXT("Reference count: %d"), PlayerStats.GetSharedReferenceCount());
		// Output: Reference count: 3

		// Modify through one system
		Modifier.ApplyDamage(25);

		// See changes in another system
		Display.Update();

		// When systems go out of scope, ref count decreases
		// When ref count reaches 0, object is automatically deleted
	}

	void NullHandlingExample()
	{
		TSharedPtr<FPlayerStats> Stats;  // Starts as null

		// Always check before use
		if (Stats.IsValid())
		{
			Stats->PrintStats();  // Won't execute
		}

		// Or use Get() which returns nullptr if invalid
		if (FPlayerStats* RawPtr = Stats.Get())
		{
			RawPtr->PrintStats();  // Won't execute
		}

		// Initialize it
		Stats = MakeShared<FPlayerStats>(TEXT("NewPlayer"));

		// Now it's valid
		check(Stats.IsValid());
		Stats->PrintStats();  // Safe

		// Reset to null
		Stats.Reset();
		check(!Stats.IsValid());
	}

	void ThreadSafetyExample()
	{
		// Default: Thread-safe (uses atomic operations)
		TSharedPtr<FPlayerStats> ThreadSafeStats = MakeShared<FPlayerStats>(TEXT("Shared"));

		// Can be safely shared across threads
		// Reference counting is atomic

		// For single-threaded scenarios, use NotThreadSafe mode for better performance
		TSharedPtr<FPlayerStats, ESPMode::NotThreadSafe> FastStats =
			MakeShared<FPlayerStats, ESPMode::NotThreadSafe>(TEXT("Fast"));

		// Faster reference counting, but NOT safe to share across threads
	}

	void PassingToFunctions()
	{
		TSharedPtr<FPlayerStats> Stats = MakeShared<FPlayerStats>(TEXT("Player"));

		// Pass by const reference to avoid unnecessary ref count changes
		ProcessStats(Stats);

		// Return shared pointers by value
		TSharedPtr<FPlayerStats> NewStats = CreateStats(TEXT("NewPlayer"));
	}

private:
	void ProcessStats(const TSharedPtr<FPlayerStats>& Stats)
	{
		// No ref count change when passing by const reference
		if (Stats.IsValid())
		{
			Stats->PrintStats();
		}
	}

	TSharedPtr<FPlayerStats> CreateStats(const FString& Name)
	{
		// Return by value - move semantics apply
		return MakeShared<FPlayerStats>(Name);
	}
};

// Example: Comparison with raw pointers
class FComparisonExample
{
public:
	void OldWay_ManualMemoryManagement()
	{
		// Manual memory management - error prone!
		FPlayerStats* Stats = new FPlayerStats(TEXT("OldStyle"));

		// Use it...
		Stats->PrintStats();

		// Must remember to delete - memory leak if forgotten!
		delete Stats;
	}

	void NewWay_AutomaticManagement()
	{
		// Automatic memory management
		TSharedPtr<FPlayerStats> Stats = MakeShared<FPlayerStats>(TEXT("ModernStyle"));

		// Use it...
		Stats->PrintStats();

		// Automatically deleted when Stats goes out of scope
		// No manual cleanup needed!
	}
};
