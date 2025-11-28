// Exercise 2: Real-World Scenarios
// Implement a game inventory and quest system using appropriate pointer types

#pragma once

#include "CoreMinimal.h"

/*
 * EXERCISE: Implement a Quest and Inventory System
 *
 * Requirements:
 * 1. Items are shared between inventory and equipped slots
 * 2. Quests have weak references to required items (quest shouldn't keep items alive)
 * 3. Quest manager uniquely owns quest progression data
 * 4. Player stats are shared between multiple systems
 * 5. Quest objectives can reference NPCs that might be destroyed
 */

// Item that can be in inventory or equipped
struct FItem
{
	FString ItemName;
	int32 Value;
	float Weight;

	FItem(const FString& Name, int32 InValue, float InWeight)
		: ItemName(Name)
		, Value(InValue)
		, Weight(InWeight)
	{
	}
};

// Quest progression data
struct FQuestProgress
{
	int32 CurrentStep;
	TArray<FString> CompletedObjectives;

	FQuestProgress()
		: CurrentStep(0)
	{
	}
};

// Quest definition
class FQuest
{
public:
	FString QuestName;
	TArray<FString> RequiredItems;

	// TODO: Add weak reference to required item instances
	// Hint: Quest shouldn't prevent items from being sold/destroyed

	explicit FQuest(const FString& Name)
		: QuestName(Name)
	{
	}

	bool CheckCompletion(/* TODO: Add parameters */)
	{
		// TODO: Implement completion check
		// - Check if required items exist
		// - Use weak pointers to verify items haven't been destroyed
		return false;
	}
};

// Player inventory system
class FInventorySystem
{
public:
	// TODO: Implement inventory using appropriate pointer type
	// Requirements:
	// - Items can be shared with equipment system
	// - Items can be referenced by quests (weakly)
	// - Multiple inventory slots can reference same item (stackable)

	// TArray<???> InventorySlots;

	void AddItem(const FString& ItemName, int32 Value, float Weight)
	{
		// TODO: Create item with appropriate pointer type
		// TODO: Add to inventory
	}

	void RemoveItem(const FString& ItemName)
	{
		// TODO: Remove item from inventory
		// Should not crash if quests are referencing it
	}

	// TODO: Return appropriate pointer type
	// ??? FindItem(const FString& ItemName)
	// {
	// 	// TODO: Implement search
	// 	return nullptr;
	// }
};

// Equipment system that shares items with inventory
class FEquipmentSystem
{
public:
	// TODO: Choose appropriate pointer types for equipped items
	// Hint: Equipped items are shared with inventory

	// ??? EquippedWeapon;
	// ??? EquippedArmor;

	void EquipItem(/* TODO: Add parameter */)
	{
		// TODO: Equip item (share reference with inventory)
	}

	void UnequipItem()
	{
		// TODO: Unequip (release reference)
	}
};

// Quest manager
class FQuestManager
{
public:
	// TODO: Choose appropriate pointer type for quest progression
	// Hint: Each quest's progression is unique to this manager

	// TMap<FString, ???> QuestProgression;

	void StartQuest(const FString& QuestName)
	{
		// TODO: Create unique quest progression
	}

	void UpdateProgress(const FString& QuestName, const FString& Objective)
	{
		// TODO: Update quest progression
	}

	// TODO: Return appropriate pointer type
	// ??? GetQuestProgress(const FString& QuestName)
	// {
	// 	// TODO: Implement
	// 	return nullptr;
	// }
};

// Player stats shared across systems
struct FPlayerStats
{
	int32 Health;
	int32 Stamina;
	float CarryWeight;
};

// Game manager that ties everything together
class FGameManager
{
public:
	// TODO: Choose appropriate pointer types for each system

	// Player stats - shared by multiple systems
	// ??? PlayerStats;

	// Inventory system - uniquely owned by game manager
	// ??? InventorySystem;

	// Equipment system - uniquely owned by game manager
	// ??? EquipmentSystem;

	// Quest manager - uniquely owned by game manager
	// ??? QuestManager;

	void Initialize()
	{
		// TODO: Initialize all systems with appropriate pointer types
		// TODO: Share player stats with systems that need it
	}

	void TestScenario()
	{
		// TODO: Implement a test scenario:
		// 1. Add some items to inventory
		// 2. Equip an item (share reference)
		// 3. Start a quest that requires an item
		// 4. Sell/remove the item (quest should detect it's gone)
		// 5. Verify systems still work correctly
	}
};

/*
 * BONUS CHALLENGES:
 *
 * 1. Add an NPC system where:
 *    - Quests have weak references to NPCs
 *    - NPCs can be destroyed during gameplay
 *    - Quest system detects when NPC is gone
 *
 * 2. Implement item stacking:
 *    - Multiple inventory slots can reference same item type
 *    - Use shared pointers appropriately
 *
 * 3. Add a save/load system:
 *    - Consider which pointer types can be serialized
 *    - How to restore references after loading?
 */

// See Exercise02_RealWorld_Solution.h for answers
