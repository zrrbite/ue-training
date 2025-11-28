# Module 1: Smart Pointers and References in Unreal Engine

## Overview

Understanding memory management in Unreal Engine is crucial. This module covers the different pointer types, when to use each, and common pitfalls to avoid.

## Table of Contents

1. [UObject Pointers and Garbage Collection](#uobject-pointers-and-garbage-collection)
2. [TSharedPtr](#tsharedptr)
3. [TSharedRef](#tsharedref)
4. [TWeakPtr](#tweakptr)
5. [TUniquePtr](#tuniqueptr)
6. [Decision Matrix: When to Use What](#decision-matrix)
7. [Common Pitfalls](#common-pitfalls)
8. [Best Practices](#best-practices)

---

## UObject Pointers and Garbage Collection

### When to Use
- **Always** for UObject-derived classes (UActorComponent, AActor, UObject, etc.)
- Objects that need to be part of Unreal's reflection system
- Objects that should be serialized/saved
- Objects that need to be exposed to Blueprints

### Key Characteristics
- Managed by Unreal's garbage collector
- Must be marked with UPROPERTY() to prevent garbage collection
- **Never manually delete** UObject pointers
- Use raw pointers (MyClass*) or TObjectPtr<MyClass> (UE5.1+)

### Example Pattern
```cpp
UCLASS()
class UMyComponent : public UActorComponent
{
    GENERATED_BODY()

    // This prevents garbage collection
    UPROPERTY()
    UObject* ManagedObject;

    // This will be garbage collected if nothing else references it!
    UObject* UnmanagedObject;  // DANGER!

    // UE 5.1+ preferred syntax
    UPROPERTY()
    TObjectPtr<UObject> ModernManagedObject;
};
```

### Caveats
- Raw UObject pointers without UPROPERTY() can become dangling
- Use TWeakObjectPtr<> for weak references to UObjects
- Cannot use std::shared_ptr or TSharedPtr with UObjects

---

## TSharedPtr

### When to Use
- **Non-UObject** types that need shared ownership
- Objects that multiple systems need to access
- Objects with unclear or shared lifetime
- Structs or plain C++ classes (not UObject-derived)

### Key Characteristics
- Reference counted (thread-safe by default)
- Automatically deletes when last reference is destroyed
- Overhead: 2 reference counts (strong + weak)
- Can be null

### Example Pattern
```cpp
// Creating a shared pointer
TSharedPtr<FMyData> DataPtr = MakeShared<FMyData>();

// Sharing ownership
TSharedPtr<FMyData> AnotherRef = DataPtr;

// Check validity
if (DataPtr.IsValid())
{
    DataPtr->DoSomething();
}

// Explicitly reset
DataPtr.Reset();
```

### Performance Considerations
- Atomic reference counting (thread-safe but has cost)
- Use `MakeShared<>()` instead of `TSharedPtr<>(new ...)` for better performance
- For single-threaded contexts, can use `ESPMode::NotThreadSafe`

---

## TSharedRef

### When to Use
- Same scenarios as TSharedPtr, but when **null is not valid**
- Function parameters where null input would be a bug
- Return values that are guaranteed to exist
- Reduces null-checking boilerplate

### Key Characteristics
- **Cannot be null** (enforced at compile time)
- Same reference counting as TSharedPtr
- Can convert to TSharedPtr, but not vice versa (safely)
- Preferred over TSharedPtr when null is not a valid state

### Example Pattern
```cpp
// Creating a shared reference - must be initialized
TSharedRef<FMyData> DataRef = MakeShared<FMyData>();

// No need for null checks!
DataRef->DoSomething();  // Always safe

// Function signature that guarantees non-null
void ProcessData(TSharedRef<FMyData> Data)
{
    // No IsValid() check needed
    Data->Process();
}

// Convert to TSharedPtr if needed
TSharedPtr<FMyData> PtrVersion = DataRef;
```

### When to Prefer Over TSharedPtr
- APIs where null input would be programmer error
- Avoiding defensive null checks in performance-critical code
- Making ownership semantics clearer

---

## TWeakPtr

### When to Use
- Breaking circular references
- Caching references without affecting lifetime
- Observer patterns
- Optional/conditional references
- When you need to check if an object still exists

### Key Characteristics
- Does **not** affect reference count
- Can become invalid (dangling)
- Must convert to TSharedPtr before use (Pin())
- Prevents memory leaks from circular dependencies

### Example Pattern
```cpp
class FObserver
{
    TWeakPtr<FSubject> SubjectWeak;

public:
    void SetSubject(TSharedPtr<FSubject> Subject)
    {
        SubjectWeak = Subject;
    }

    void CheckSubject()
    {
        // Pin() converts to TSharedPtr - null if object destroyed
        if (TSharedPtr<FSubject> Subject = SubjectWeak.Pin())
        {
            Subject->DoSomething();
        }
        else
        {
            // Subject has been destroyed
        }
    }
};
```

### Common Use Cases
```cpp
// 1. Breaking circular references
class FParent
{
    TSharedPtr<FChild> Child;  // Strong reference down
};

class FChild
{
    TWeakPtr<FParent> Parent;  // Weak reference up - breaks cycle!
};

// 2. Caching without ownership
class FCache
{
    TMap<FName, TWeakPtr<FExpensiveObject>> CachedObjects;

    TSharedPtr<FExpensiveObject> GetOrCreate(FName Key)
    {
        if (TSharedPtr<FExpensiveObject> Cached = CachedObjects[Key].Pin())
        {
            return Cached;  // Still alive
        }

        // Create new and cache weakly
        TSharedPtr<FExpensiveObject> NewObj = MakeShared<FExpensiveObject>();
        CachedObjects[Key] = NewObj;
        return NewObj;
    }
};
```

---

## TUniquePtr

### When to Use
- **Exclusive ownership** (only one owner)
- RAII (Resource Acquisition Is Initialization) patterns
- Replacing raw pointers with clear ownership
- Implementing Pimpl idiom
- Non-copyable resources

### Key Characteristics
- No reference counting overhead
- Cannot be copied, only moved
- Automatically deletes on destruction
- Zero overhead compared to raw pointer
- Works with custom deleters

### Example Pattern
```cpp
// Creating unique pointer
TUniquePtr<FMyData> DataPtr = MakeUnique<FMyData>();

// Move semantics only
TUniquePtr<FMyData> Moved = MoveTemp(DataPtr);
// DataPtr is now null!

// Custom deleter
TUniquePtr<FFile> FilePtr = MakeUnique<FFile>();
// Automatically closed when FilePtr goes out of scope

// Arrays
TUniquePtr<int32[]> ArrayPtr = MakeUnique<int32[]>(100);
```

### When to Prefer Over TSharedPtr
- Clear single ownership model
- Performance-critical code (no atomic operations)
- Temporary/local objects
- Private implementation details

---

## Decision Matrix

| Scenario | Use This | Why |
|----------|----------|-----|
| UObject-derived class | Raw pointer + UPROPERTY | Garbage collector manages lifetime |
| Weak reference to UObject | TWeakObjectPtr | Safe weak reference for GC'd objects |
| Shared ownership, non-UObject | TSharedPtr | Multiple owners, ref counted |
| Shared ownership, never null | TSharedRef | Compile-time null safety |
| Breaking circular references | TWeakPtr | Doesn't affect ref count |
| Exclusive ownership | TUniquePtr | Single owner, zero overhead |
| Optional reference without ownership | TWeakPtr | Can check validity, no lifetime impact |
| Function parameter (won't store) | const FMyType& | No pointer overhead |
| Function parameter (guarantees non-null) | TSharedRef | Clear contract |

---

## Common Pitfalls

### ❌ DON'T: Use TSharedPtr with UObjects
```cpp
// WRONG! UObjects should not use shared pointers
TSharedPtr<UMyObject> BadPtr = MakeShared<UMyObject>();
```

### ❌ DON'T: Forget UPROPERTY on UObject pointers
```cpp
UCLASS()
class UMyClass : public UObject
{
    // WRONG! Will be garbage collected unexpectedly
    UObject* MyObject;

    // CORRECT!
    UPROPERTY()
    UObject* MyManagedObject;
};
```

### ❌ DON'T: Create circular TSharedPtr references
```cpp
// WRONG! Memory leak - neither can be destroyed
class FNodeA
{
    TSharedPtr<FNodeB> Next;
};

class FNodeB
{
    TSharedPtr<FNodeA> Prev;  // Should be TWeakPtr!
};
```

### ❌ DON'T: Store TSharedRef when null is possible
```cpp
// WRONG! What if factory returns null?
TSharedRef<FData> Data = Factory->Create();  // Crash if Create() returns null

// CORRECT!
TSharedPtr<FData> Data = Factory->Create();
if (Data.IsValid()) { /* ... */ }
```

### ❌ DON'T: Dereference TWeakPtr directly
```cpp
TWeakPtr<FData> WeakData;

// WRONG! Compiler error
WeakData->DoSomething();

// CORRECT!
if (TSharedPtr<FData> Data = WeakData.Pin())
{
    Data->DoSomething();
}
```

### ❌ DON'T: Copy TUniquePtr
```cpp
TUniquePtr<FData> Data1 = MakeUnique<FData>();
TUniquePtr<FData> Data2 = Data1;  // WRONG! Compiler error

// CORRECT! Use move semantics
TUniquePtr<FData> Data2 = MoveTemp(Data1);
```

---

## Best Practices

### 1. Prefer TSharedRef Over TSharedPtr When Possible
```cpp
// Good: Clear that null is not valid
TSharedRef<FConfig> LoadConfig()
{
    return MakeShared<FConfig>();
}

// Less clear: Caller must check for null
TSharedPtr<FConfig> LoadConfig()
{
    return MakeShared<FConfig>();
}
```

### 2. Use MakeShared/MakeUnique
```cpp
// Preferred: Single allocation
TSharedPtr<FData> Good = MakeShared<FData>();

// Avoid: Two allocations (object + control block)
TSharedPtr<FData> Less = TSharedPtr<FData>(new FData());
```

### 3. Break Cycles with TWeakPtr
```cpp
// Parent-child relationships
class FParent
{
    TArray<TSharedPtr<FChild>> Children;  // Strong down
};

class FChild
{
    TWeakPtr<FParent> Parent;  // Weak up
};
```

### 4. Use Forward Declarations with TUniquePtr
```cpp
// Header file
class FImpl;  // Forward declare

class FMyClass
{
    TUniquePtr<FImpl> Impl;  // Pimpl pattern
public:
    FMyClass();
    ~FMyClass();  // Must be defined in .cpp where FImpl is complete
};
```

### 5. Const Correctness
```cpp
void ProcessData(const TSharedRef<const FData>& Data)
{
    // Data cannot be modified or reassigned
}
```

### 6. Thread Safety Considerations
```cpp
// Default: Thread-safe (slower)
TSharedPtr<FData> ThreadSafePtr = MakeShared<FData>();

// Single-threaded optimization (faster)
TSharedPtr<FData, ESPMode::NotThreadSafe> FastPtr =
    MakeShared<FData, ESPMode::NotThreadSafe>();
```

---

## Next Steps

1. Review the examples in `Examples/` directory
2. Complete the exercises in `Exercises/` directory
3. Experiment with the different pointer types in a test project
4. Move on to Module 2: Task System (TTask)

## Additional Resources

- [Unreal Engine Documentation: Smart Pointers](https://docs.unrealengine.com/en-US/ProgrammingAndScripting/ProgrammingWithCPP/SmartPointerLibrary/)
- [Unreal Engine Documentation: Garbage Collection](https://docs.unrealengine.com/en-US/ProgrammingAndScripting/ProgrammingWithCPP/UnrealArchitecture/Objects/Optimizations/)
