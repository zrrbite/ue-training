// Exercise 2: Real-World Scenarios - Solution

#pragma once

#include "CoreMinimal.h"

/*
 * SOLUTION: Complete Quest and Inventory System Implementation
 */

// Item that can be in inventory or equipped
struct FItem_Sol
{
	FString ItemName;
	int32 Value;
	float Weight;

	FItem_Sol(const FString& Name, int32 InValue, float InWeight)
		: ItemName(Name)
		, Value(InValue)
		, Weight(InWeight)
	{
		UE_LOG(LogTemp, Log, TEXT("Item created: %s"), *ItemName);
	}

	~FItem_Sol()
	{
		UE_LOG(LogTemp, Log, TEXT("Item destroyed: %s"), *ItemName);
	}
};

// Quest progression data - uniquely owned
struct FQuestProgress_Sol
{
	int32 CurrentStep;
	TArray<FString> CompletedObjectives;

	FQuestProgress_Sol()
		: CurrentStep(0)
	{
	}
};

// Quest definition
class FQuest_Sol
{
public:
	FString QuestName;
	TArray<FString> RequiredItemNames;

	// SOLUTION: Weak pointers - quest doesn't keep items alive
	TArray<TWeakPtr<FItem_Sol>> RequiredItems;

	explicit FQuest_Sol(const FString& Name)
		: QuestName(Name)
	{
	}

	void AddRequiredItem(const FString& ItemName, TWeakPtr<FItem_Sol> Item)
	{
		RequiredItemNames.Add(ItemName);
		RequiredItems.Add(Item);
	}

	bool CheckCompletion() const
	{
		// Check if all required items still exist
		for (int32 i = 0; i < RequiredItems.Num(); i++)
		{
			if (TSharedPtr<FItem_Sol> Item = RequiredItems[i].Pin())
			{
				UE_LOG(LogTemp, Log, TEXT("Quest '%s': Item '%s' found"),
					*QuestName, *Item->ItemName);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Quest '%s': Required item '%s' is missing!"),
					*QuestName, *RequiredItemNames[i]);
				return false;
			}
		}

		return true;
	}
};

// Player inventory system
class FInventorySystem_Sol
{
public:
	// SOLUTION: TSharedPtr - items are shared with equipment and quests
	TArray<TSharedPtr<FItem_Sol>> InventorySlots;

	void AddItem(const FString& ItemName, int32 Value, float Weight)
	{
		// Create shared item
		TSharedPtr<FItem_Sol> NewItem = MakeShared<FItem_Sol>(ItemName, Value, Weight);
		InventorySlots.Add(NewItem);

		UE_LOG(LogTemp, Log, TEXT("Added to inventory: %s (Total items: %d)"),
			*ItemName, InventorySlots.Num());
	}

	void RemoveItem(const FString& ItemName)
	{
		int32 RemovedCount = InventorySlots.RemoveAll([&ItemName](const TSharedPtr<FItem_Sol>& Item)
		{
			return Item.IsValid() && Item->ItemName == ItemName;
		});

		if (RemovedCount > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Removed from inventory: %s"), *ItemName);
			// If quests were holding weak references, they'll detect item is gone
		}
	}

	// SOLUTION: Return TSharedPtr for sharing
	TSharedPtr<FItem_Sol> FindItem(const FString& ItemName) const
	{
		for (const TSharedPtr<FItem_Sol>& Item : InventorySlots)
		{
			if (Item.IsValid() && Item->ItemName == ItemName)
			{
				return Item;
			}
		}
		return nullptr;
	}

	int32 GetItemCount() const
	{
		return InventorySlots.Num();
	}
};

// Equipment system that shares items with inventory
class FEquipmentSystem_Sol
{
public:
	// SOLUTION: TSharedPtr - shares items with inventory
	TSharedPtr<FItem_Sol> EquippedWeapon;
	TSharedPtr<FItem_Sol> EquippedArmor;

	void EquipWeapon(TSharedPtr<FItem_Sol> Item)
	{
		if (Item.IsValid())
		{
			EquippedWeapon = Item;  // Share reference
			UE_LOG(LogTemp, Log, TEXT("Equipped weapon: %s (RefCount: %d)"),
				*Item->ItemName, Item.GetSharedReferenceCount());
		}
	}

	void UnequipWeapon()
	{
		if (EquippedWeapon.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("Unequipped weapon: %s"), *EquippedWeapon->ItemName);
			EquippedWeapon.Reset();  // Release reference
		}
	}

	bool HasWeaponEquipped() const
	{
		return EquippedWeapon.IsValid();
	}
};

// Quest manager
class FQuestManager_Sol
{
public:
	// SOLUTION: TUniquePtr - each quest progression is uniquely owned
	TMap<FString, TUniquePtr<FQuestProgress_Sol>> QuestProgression;

	// Active quests
	TArray<TSharedPtr<FQuest_Sol>> ActiveQuests;

	TSharedPtr<FQuest_Sol> StartQuest(const FString& QuestName)
	{
		// Create unique quest progression
		QuestProgression.Add(QuestName, MakeUnique<FQuestProgress_Sol>());

		// Create shared quest (can be referenced by UI, etc.)
		TSharedPtr<FQuest_Sol> NewQuest = MakeShared<FQuest_Sol>(QuestName);
		ActiveQuests.Add(NewQuest);

		UE_LOG(LogTemp, Log, TEXT("Started quest: %s"), *QuestName);
		return NewQuest;
	}

	void UpdateProgress(const FString& QuestName, const FString& Objective)
	{
		if (TUniquePtr<FQuestProgress_Sol>* Progress = QuestProgression.Find(QuestName))
		{
			(*Progress)->CompletedObjectives.Add(Objective);
			(*Progress)->CurrentStep++;

			UE_LOG(LogTemp, Log, TEXT("Quest '%s' updated: %s (Step %d)"),
				*QuestName, *Objective, (*Progress)->CurrentStep);
		}
	}

	FQuestProgress_Sol* GetQuestProgress(const FString& QuestName)
	{
		if (TUniquePtr<FQuestProgress_Sol>* Progress = QuestProgression.Find(QuestName))
		{
			return Progress->Get();  // Return raw pointer for temporary access
		}
		return nullptr;
	}

	void CheckAllQuests()
	{
		for (TSharedPtr<FQuest_Sol>& Quest : ActiveQuests)
		{
			if (Quest.IsValid())
			{
				bool bComplete = Quest->CheckCompletion();
				UE_LOG(LogTemp, Log, TEXT("Quest '%s' complete: %s"),
					*Quest->QuestName, bComplete ? TEXT("Yes") : TEXT("No"));
			}
		}
	}
};

// Player stats shared across systems
struct FPlayerStats_Sol
{
	int32 Health;
	int32 Stamina;
	float CarryWeight;
	float CurrentWeight;

	FPlayerStats_Sol()
		: Health(100)
		, Stamina(100)
		, CarryWeight(100.0f)
		, CurrentWeight(0.0f)
	{
	}
};

// Game manager that ties everything together
class FGameManager_Sol
{
public:
	// SOLUTION: TSharedPtr - stats shared by multiple systems
	TSharedPtr<FPlayerStats_Sol> PlayerStats;

	// SOLUTION: TUniquePtr - systems are uniquely owned
	TUniquePtr<FInventorySystem_Sol> InventorySystem;
	TUniquePtr<FEquipmentSystem_Sol> EquipmentSystem;
	TUniquePtr<FQuestManager_Sol> QuestManager;

	void Initialize()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Initializing Game Manager ==="));

		// Create shared player stats
		PlayerStats = MakeShared<FPlayerStats_Sol>();

		// Create unique systems
		InventorySystem = MakeUnique<FInventorySystem_Sol>();
		EquipmentSystem = MakeUnique<FEquipmentSystem_Sol>();
		QuestManager = MakeUnique<FQuestManager_Sol>();

		UE_LOG(LogTemp, Log, TEXT("All systems initialized"));
	}

	void TestScenario()
	{
		if (!InventorySystem || !EquipmentSystem || !QuestManager)
		{
			UE_LOG(LogTemp, Error, TEXT("Systems not initialized!"));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("\n=== Test Scenario: Quest Item Management ===\n"));

		// 1. Add items to inventory
		UE_LOG(LogTemp, Log, TEXT("--- Step 1: Adding items ---"));
		InventorySystem->AddItem(TEXT("Iron Sword"), 100, 5.0f);
		InventorySystem->AddItem(TEXT("Health Potion"), 25, 0.5f);
		InventorySystem->AddItem(TEXT("Dragon Scale"), 500, 1.0f);

		// 2. Equip an item (share reference)
		UE_LOG(LogTemp, Log, TEXT("\n--- Step 2: Equipping weapon ---"));
		TSharedPtr<FItem_Sol> Sword = InventorySystem->FindItem(TEXT("Iron Sword"));
		if (Sword.IsValid())
		{
			EquipmentSystem->EquipWeapon(Sword);
			UE_LOG(LogTemp, Log, TEXT("Sword ref count: %d (Inventory + Equipment)"),
				Sword.GetSharedReferenceCount());
		}

		// 3. Start a quest that requires dragon scale
		UE_LOG(LogTemp, Log, TEXT("\n--- Step 3: Starting quest ---"));
		TSharedPtr<FQuest_Sol> DragonQuest = QuestManager->StartQuest(TEXT("Slay the Dragon"));

		TSharedPtr<FItem_Sol> DragonScale = InventorySystem->FindItem(TEXT("Dragon Scale"));
		if (DragonScale.IsValid())
		{
			DragonQuest->AddRequiredItem(TEXT("Dragon Scale"), DragonScale);  // Weak reference
			UE_LOG(LogTemp, Log, TEXT("Dragon Scale ref count: %d (Only inventory has strong ref)"),
				DragonScale.GetSharedReferenceCount());
		}

		// Check quest - should pass
		UE_LOG(LogTemp, Log, TEXT("\n--- Step 4: Checking quest (should pass) ---"));
		QuestManager->CheckAllQuests();

		// 4. Sell/remove the dragon scale
		UE_LOG(LogTemp, Log, TEXT("\n--- Step 5: Selling dragon scale ---"));
		InventorySystem->RemoveItem(TEXT("Dragon Scale"));

		// Check quest - should fail now
		UE_LOG(LogTemp, Log, TEXT("\n--- Step 6: Checking quest (should fail) ---"));
		QuestManager->CheckAllQuests();

		// 5. Verify sword is still equipped and referenced by both systems
		UE_LOG(LogTemp, Log, TEXT("\n--- Step 7: Verifying sword still works ---"));
		if (EquipmentSystem->HasWeaponEquipped() && Sword.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("Sword still equipped: %s (RefCount: %d)"),
				*Sword->ItemName, Sword.GetSharedReferenceCount());
		}

		// 6. Unequip and remove sword
		UE_LOG(LogTemp, Log, TEXT("\n--- Step 8: Unequipping and removing sword ---"));
		EquipmentSystem->UnequipWeapon();
		InventorySystem->RemoveItem(TEXT("Iron Sword"));
		// Sword should be destroyed now (no more references)

		UE_LOG(LogTemp, Log, TEXT("\n=== Test Complete ===\n"));
	}
};

// Test function
inline void RunGameManagerTest()
{
	FGameManager_Sol GameManager;
	GameManager.Initialize();
	GameManager.TestScenario();
}

/*
 * Key Takeaways from This Solution:
 *
 * 1. TSharedPtr<FItem> - Items are shared between inventory, equipment, and other systems
 * 2. TWeakPtr<FItem> in Quests - Quests don't prevent items from being removed
 * 3. TUniquePtr<FQuestProgress> - Each quest's progression is uniquely owned by manager
 * 4. TSharedPtr<FPlayerStats> - Stats shared across multiple systems
 * 5. TUniquePtr<Systems> - Game manager uniquely owns all subsystems
 *
 * This demonstrates real-world patterns you'll use in game development!
 */
