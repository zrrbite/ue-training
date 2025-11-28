// Example 6: Real-World Combined Example
// Demonstrates using multiple pointer types together in a realistic scenario

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "06_RealWorld_Combined.generated.h"

// Non-UObject data structures - use smart pointers

struct FPlayerStats
{
	int32 Health;
	int32 MaxHealth;
	float Stamina;

	FPlayerStats()
		: Health(100)
		, MaxHealth(100)
		, Stamina(100.0f)
	{
	}
};

struct FInventoryData
{
	TArray<FString> Items;
	int32 Gold;

	FInventoryData()
		: Gold(0)
	{
	}

	void AddItem(const FString& Item)
	{
		Items.Add(Item);
		UE_LOG(LogTemp, Log, TEXT("Added item: %s (Total: %d items)"), *Item, Items.Num());
	}
};

// Configuration - always valid, shared across systems
struct FGameplayConfig
{
	float DamageMultiplier;
	float HealthRegenRate;
	bool bHardcoreMode;

	FGameplayConfig()
		: DamageMultiplier(1.0f)
		, HealthRegenRate(0.5f)
		, bHardcoreMode(false)
	{
	}
};

// Game systems using appropriate pointer types

class FStatsSystem
{
private:
	TSharedRef<FGameplayConfig> Config;  // Always needs valid config
	TSharedPtr<FPlayerStats> Stats;       // Shared with other systems

public:
	FStatsSystem(TSharedRef<FGameplayConfig> InConfig, TSharedPtr<FPlayerStats> InStats)
		: Config(InConfig)
		, Stats(InStats)
	{
	}

	void ApplyDamage(int32 Damage)
	{
		if (Stats.IsValid())
		{
			int32 ActualDamage = Damage * Config->DamageMultiplier;
			Stats->Health = FMath::Max(0, Stats->Health - ActualDamage);

			UE_LOG(LogTemp, Log, TEXT("Applied %d damage. Health: %d/%d"),
				ActualDamage, Stats->Health, Stats->MaxHealth);
		}
	}

	void RegenerateHealth(float DeltaTime)
	{
		if (Stats.IsValid() && Stats->Health < Stats->MaxHealth)
		{
			int32 RegenAmount = Config->HealthRegenRate * DeltaTime;
			Stats->Health = FMath::Min(Stats->MaxHealth, Stats->Health + RegenAmount);
		}
	}
};

class FInventorySystem
{
private:
	TUniquePtr<FInventoryData> Inventory;  // Exclusive ownership
	TWeakPtr<FPlayerStats> PlayerStats;     // Optional reference, no ownership

public:
	FInventorySystem()
		: Inventory(MakeUnique<FInventoryData>())
	{
	}

	void SetPlayerStats(TSharedPtr<FPlayerStats> Stats)
	{
		PlayerStats = Stats;
	}

	void AddItem(const FString& ItemName)
	{
		Inventory->AddItem(ItemName);
	}

	void UseHealthPotion()
	{
		// Check if we have the item
		int32 PotionIndex = Inventory->Items.Find(TEXT("Health Potion"));
		if (PotionIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("No health potions available"));
			return;
		}

		// Use weak pointer to access stats
		if (TSharedPtr<FPlayerStats> Stats = PlayerStats.Pin())
		{
			Stats->Health = FMath::Min(Stats->MaxHealth, Stats->Health + 50);
			Inventory->Items.RemoveAt(PotionIndex);
			UE_LOG(LogTemp, Log, TEXT("Used health potion. Health: %d/%d"),
				Stats->Health, Stats->MaxHealth);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot use potion - no player stats"));
		}
	}

	int32 GetGold() const
	{
		return Inventory->Gold;
	}

	void AddGold(int32 Amount)
	{
		Inventory->Gold += Amount;
	}
};

// Observer pattern for UI updates
class IStatsObserver
{
public:
	virtual ~IStatsObserver() = default;
	virtual void OnStatsChanged(const FPlayerStats& Stats) = 0;
};

class FStatsUIWidget : public IStatsObserver
{
public:
	FString WidgetName;

	explicit FStatsUIWidget(const FString& Name)
		: WidgetName(Name)
	{
	}

	void OnStatsChanged(const FPlayerStats& Stats) override
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] HP: %d/%d | Stamina: %.1f"),
			*WidgetName, Stats.Health, Stats.MaxHealth, Stats.Stamina);
	}
};

class FObservableStats
{
private:
	TSharedPtr<FPlayerStats> Stats;
	TArray<TWeakPtr<IStatsObserver>> Observers;  // Weak - don't keep UI alive

public:
	explicit FObservableStats(TSharedPtr<FPlayerStats> InStats)
		: Stats(InStats)
	{
	}

	void AddObserver(TWeakPtr<IStatsObserver> Observer)
	{
		Observers.Add(Observer);
	}

	void ModifyHealth(int32 Delta)
	{
		if (Stats.IsValid())
		{
			Stats->Health = FMath::Clamp(Stats->Health + Delta, 0, Stats->MaxHealth);
			NotifyObservers();
		}
	}

	void NotifyObservers()
	{
		// Clean up dead observers
		Observers.RemoveAll([](const TWeakPtr<IStatsObserver>& Weak)
		{
			return !Weak.IsValid();
		});

		// Notify living observers
		for (TWeakPtr<IStatsObserver>& WeakObserver : Observers)
		{
			if (TSharedPtr<IStatsObserver> Observer = WeakObserver.Pin())
			{
				Observer->OnStatsChanged(*Stats);
			}
		}
	}
};

// UObject Actor that ties it all together
UCLASS()
class AGameplayManager : public AActor
{
	GENERATED_BODY()

private:
	// Shared configuration across all systems
	TSharedRef<FGameplayConfig> Config;

	// Shared player stats
	TSharedPtr<FPlayerStats> PlayerStats;

	// Individual systems with appropriate ownership
	TUniquePtr<FStatsSystem> StatsSystem;
	TUniquePtr<FInventorySystem> InventorySystem;
	TSharedPtr<FObservableStats> ObservableStats;

	// UObject references for Unreal objects
	UPROPERTY()
	TObjectPtr<AActor> PlayerActor;

public:
	AGameplayManager()
		: Config(MakeShared<FGameplayConfig>())  // Create shared config
	{
		PrimaryActorTick.bCanEverTick = true;
	}

	void BeginPlay() override
	{
		Super::BeginPlay();

		// Initialize shared stats
		PlayerStats = MakeShared<FPlayerStats>();

		// Create systems with appropriate pointer types
		StatsSystem = MakeUnique<FStatsSystem>(Config, PlayerStats);
		InventorySystem = MakeUnique<FInventorySystem>();
		InventorySystem->SetPlayerStats(PlayerStats);

		// Create observable stats for UI
		ObservableStats = MakeShared<FObservableStats>(PlayerStats);

		// Simulate UI widgets observing stats
		TSharedPtr<FStatsUIWidget> HealthBar = MakeShared<FStatsUIWidget>(TEXT("HealthBar"));
		TSharedPtr<FStatsUIWidget> StatusPanel = MakeShared<FStatsUIWidget>(TEXT("StatusPanel"));

		ObservableStats->AddObserver(HealthBar);
		ObservableStats->AddObserver(StatusPanel);

		// Initial setup
		InventorySystem->AddItem(TEXT("Sword"));
		InventorySystem->AddItem(TEXT("Health Potion"));
		InventorySystem->AddGold(100);

		// Test damage
		UE_LOG(LogTemp, Log, TEXT("=== Taking Damage ==="));
		StatsSystem->ApplyDamage(30);
		ObservableStats->NotifyObservers();

		// Test potion
		UE_LOG(LogTemp, Log, TEXT("=== Using Health Potion ==="));
		InventorySystem->UseHealthPotion();
		ObservableStats->NotifyObservers();

		// Demonstrate config changes affect all systems
		UE_LOG(LogTemp, Log, TEXT("=== Enabling Hardcore Mode ==="));
		Config->DamageMultiplier = 2.0f;
		Config->bHardcoreMode = true;

		StatsSystem->ApplyDamage(20);  // Now does 40 damage
		ObservableStats->NotifyObservers();
	}

	void Tick(float DeltaTime) override
	{
		Super::Tick(DeltaTime);

		// Systems can access shared data
		if (StatsSystem)
		{
			StatsSystem->RegenerateHealth(DeltaTime);
		}
	}
};

// Summary of pointer usage in this example:
//
// TSharedRef<FGameplayConfig>: Config always needed, shared across systems
// TSharedPtr<FPlayerStats>: Stats shared between multiple systems
// TUniquePtr<FInventoryData>: Inventory exclusively owned by InventorySystem
// TUniquePtr<FStatsSystem>: System exclusively owned by manager
// TWeakPtr<IStatsObserver>: UI observers don't prevent UI deletion
// TWeakPtr<FPlayerStats>: Inventory has optional reference to stats
// TObjectPtr<AActor>: UObject reference (Unreal's GC system)
