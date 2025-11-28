// Example 4: TWeakPtr - Non-Owning References
// Use for breaking circular references and optional references

#pragma once

#include "CoreMinimal.h"

// Forward declarations
struct FInventoryItem;
class FPlayer;

// Example 1: Breaking Circular References
struct FInventoryItem
{
	FString ItemName;
	int32 Quantity;

	// Weak pointer back to owner - doesn't affect owner's lifetime
	TWeakPtr<FPlayer> Owner;

	FInventoryItem(const FString& Name, int32 Qty)
		: ItemName(Name), Quantity(Qty)
	{
	}

	void UseItem()
	{
		// Must "Pin" to convert weak pointer to shared pointer
		if (TSharedPtr<FPlayer> OwnerPtr = Owner.Pin())
		{
			UE_LOG(LogTemp, Log, TEXT("%s used %s"), *OwnerPtr->PlayerName, *ItemName);
			Quantity--;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Item has no owner (owner was deleted)"));
		}
	}
};

class FPlayer
{
public:
	FString PlayerName;

	// Strong pointers to owned items
	TArray<TSharedPtr<FInventoryItem>> Inventory;

	explicit FPlayer(const FString& Name)
		: PlayerName(Name)
	{
	}

	void AddItem(const FString& ItemName, int32 Quantity)
	{
		TSharedPtr<FInventoryItem> Item = MakeShared<FInventoryItem>(ItemName, Quantity);

		// Item holds weak reference back to player - breaks the cycle!
		Item->Owner = AsShared();

		Inventory.Add(Item);
	}

	// Required for AsShared() to work
	TSharedRef<FPlayer> AsShared()
	{
		// In real code, FPlayer would inherit from TSharedFromThis
		// This is simplified for the example
		return MakeShared<FPlayer>(PlayerName).ToSharedRef();
	}
};

// Example 2: Observer Pattern
class FSubject
{
public:
	FString Data;

	void NotifyObservers()
	{
		UE_LOG(LogTemp, Log, TEXT("Notifying observers of data change: %s"), *Data);

		// Weak pointers allow observers to be destroyed without updating this list
		CleanupDeadObservers();

		for (TWeakPtr<class FObserver>& WeakObserver : Observers)
		{
			if (TSharedPtr<class FObserver> Observer = WeakObserver.Pin())
			{
				Observer->OnNotify(Data);
			}
		}
	}

	void AddObserver(TWeakPtr<class FObserver> Observer)
	{
		Observers.Add(Observer);
	}

private:
	TArray<TWeakPtr<class FObserver>> Observers;

	void CleanupDeadObservers()
	{
		// Remove observers that have been destroyed
		Observers.RemoveAll([](const TWeakPtr<class FObserver>& Weak)
		{
			return !Weak.IsValid();
		});
	}
};

class FObserver
{
public:
	FString ObserverName;

	explicit FObserver(const FString& Name)
		: ObserverName(Name)
	{
	}

	void OnNotify(const FString& Data)
	{
		UE_LOG(LogTemp, Log, TEXT("Observer '%s' received: %s"), *ObserverName, *Data);
	}
};

// Example 3: Caching Without Ownership
class FResourceCache
{
private:
	struct FCacheEntry
	{
		FString ResourceName;
		TWeakPtr<class FResource> WeakResource;
	};

	TArray<FCacheEntry> Cache;

public:
	TSharedPtr<class FResource> GetOrLoad(const FString& ResourceName)
	{
		// Check cache first
		for (FCacheEntry& Entry : Cache)
		{
			if (Entry.ResourceName == ResourceName)
			{
				// Try to get cached resource
				if (TSharedPtr<class FResource> Cached = Entry.WeakResource.Pin())
				{
					UE_LOG(LogTemp, Log, TEXT("Cache hit: %s"), *ResourceName);
					return Cached;
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("Cache miss (resource deleted): %s"), *ResourceName);
				}
			}
		}

		// Not in cache or was deleted, load new one
		TSharedPtr<class FResource> NewResource = LoadResource(ResourceName);

		// Cache it weakly - doesn't prevent deletion when no longer used
		FCacheEntry NewEntry;
		NewEntry.ResourceName = ResourceName;
		NewEntry.WeakResource = NewResource;
		Cache.Add(NewEntry);

		return NewResource;
	}

private:
	TSharedPtr<class FResource> LoadResource(const FString& Name)
	{
		UE_LOG(LogTemp, Log, TEXT("Loading resource: %s"), *Name);
		return MakeShared<class FResource>(Name);
	}
};

class FResource
{
public:
	FString Name;

	explicit FResource(const FString& InName)
		: Name(InName)
	{
		UE_LOG(LogTemp, Log, TEXT("Resource created: %s"), *Name);
	}

	~FResource()
	{
		UE_LOG(LogTemp, Log, TEXT("Resource destroyed: %s"), *Name);
	}
};

// Example 4: Parent-Child Relationships
class FSceneNode
{
public:
	FString NodeName;

	// Strong references to children
	TArray<TSharedPtr<FSceneNode>> Children;

	// Weak reference to parent - prevents circular reference
	TWeakPtr<FSceneNode> Parent;

	explicit FSceneNode(const FString& Name)
		: NodeName(Name)
	{
	}

	void AddChild(TSharedPtr<FSceneNode> Child)
	{
		Child->Parent = AsShared();
		Children.Add(Child);
	}

	void PrintHierarchy(int32 Depth = 0)
	{
		FString Indent = FString::ChrN(Depth * 2, ' ');
		UE_LOG(LogTemp, Log, TEXT("%s- %s"), *Indent, *NodeName);

		for (TSharedPtr<FSceneNode>& Child : Children)
		{
			Child->PrintHierarchy(Depth + 1);
		}
	}

	TSharedPtr<FSceneNode> GetRoot()
	{
		// Navigate up to root using weak pointers
		TSharedPtr<FSceneNode> Current = AsShared();

		while (TSharedPtr<FSceneNode> ParentPtr = Current->Parent.Pin())
		{
			Current = ParentPtr;
		}

		return Current;
	}

	TSharedRef<FSceneNode> AsShared()
	{
		// Simplified - in real code, inherit from TSharedFromThis
		return MakeShared<FSceneNode>(NodeName).ToSharedRef();
	}
};

// Example usage
class FWeakPtrExamples
{
public:
	void CircularReferenceExample()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Circular Reference Example ==="));

		// Without weak pointers, this would leak memory
		TSharedPtr<FSceneNode> Root = MakeShared<FSceneNode>(TEXT("Root"));
		TSharedPtr<FSceneNode> Child = MakeShared<FSceneNode>(TEXT("Child"));

		Root->AddChild(Child);  // Child->Parent is weak, no cycle!

		// When Root goes out of scope, everything is cleaned up properly
	}

	void ObserverExample()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Observer Pattern Example ==="));

		FSubject Subject;

		{
			// Observers in inner scope
			TSharedPtr<FObserver> Observer1 = MakeShared<FObserver>(TEXT("Observer1"));
			TSharedPtr<FObserver> Observer2 = MakeShared<FObserver>(TEXT("Observer2"));

			Subject.AddObserver(Observer1);
			Subject.AddObserver(Observer2);

			Subject.Data = TEXT("First notification");
			Subject.NotifyObservers();
			// Both observers notified

		}  // Observers destroyed here

		Subject.Data = TEXT("Second notification");
		Subject.NotifyObservers();
		// No observers - weak pointers detect this safely
	}

	void CacheExample()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Cache Example ==="));

		FResourceCache Cache;

		// First load
		TSharedPtr<FResource> Resource1 = Cache.GetOrLoad(TEXT("Texture.png"));

		// Cache hit - same instance
		TSharedPtr<FResource> Resource2 = Cache.GetOrLoad(TEXT("Texture.png"));

		check(Resource1 == Resource2);  // Same object

		// Release references
		Resource1.Reset();
		Resource2.Reset();
		// Resource is destroyed (no longer in use)

		// Next load creates new instance (cache entry was weak)
		TSharedPtr<FResource> Resource3 = Cache.GetOrLoad(TEXT("Texture.png"));
		// "Loading resource: Texture.png" - new instance
	}

	void ValidityCheckExample()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Validity Check Example ==="));

		TWeakPtr<FResource> WeakResource;

		{
			TSharedPtr<FResource> StrongResource = MakeShared<FResource>(TEXT("Temp"));
			WeakResource = StrongResource;

			// While strong reference exists
			if (WeakResource.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("Resource is valid"));

				if (TSharedPtr<FResource> Pinned = WeakResource.Pin())
				{
					UE_LOG(LogTemp, Log, TEXT("Accessed: %s"), *Pinned->Name);
				}
			}

		}  // StrongResource destroyed here

		// After strong reference is gone
		if (!WeakResource.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("Resource is no longer valid"));
		}

		// Pin returns null
		TSharedPtr<FResource> Pinned = WeakResource.Pin();
		check(Pinned == nullptr);
	}
};
