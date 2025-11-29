# Unreal Engine Training Program

This repository contains comprehensive training materials for mastering Unreal Engine C++ concepts through practical examples and hands-on exercises.

## Training Modules

### Module 1: Smart Pointers and References in Unreal Engine
**Status:** ✅ Complete

Master memory management in Unreal Engine with comprehensive coverage of:
- UObject pointers and garbage collection
- TSharedPtr, TSharedRef, TWeakPtr, TUniquePtr
- When to use each pointer type
- Common pitfalls and best practices
- Real-world game system examples

**Location:** `Module01_SmartPointers/`

**Contents:**
- Comprehensive README with theory and decision matrices
- 6 detailed example files with working code
- 2 exercise sets with complete solutions
- Real-world quest/inventory system implementation

### Module 2: Task System (Tasks::FTask and AsyncTask)
**Status:** ✅ Complete

Master asynchronous and parallel programming in Unreal Engine:
- Tasks::FTask (UE5 modern API - recommended)
- AsyncTask system (legacy but still fully supported)
- Migration guide from AsyncTask to Tasks::FTask
- Task dependencies and chaining
- Game thread safety and UObject access
- Performance optimization and batching
- Common parallel patterns

**Note:** AsyncTask is NOT deprecated - it's fully supported in UE5, but Epic recommends Tasks::FTask for new code due to better features (dependencies, return values, debugging).

**Location:** `Module02_TaskSystem/`

**Contents:**
- Comprehensive README with thread safety rules
- 4 detailed example files covering all patterns
- Exercise set with 5 challenging problems and solutions
- Parallel algorithms (map-reduce, producer-consumer, pipeline)

## Repository Structure

```
Module01_SmartPointers/
├── README.md                          # Complete guide to smart pointers
├── Examples/
│   ├── 01_UObjectPointers.h          # GC and UObject management
│   ├── 02_TSharedPtr.h               # Shared ownership patterns
│   ├── 03_TSharedRef.h               # Non-null shared references
│   ├── 04_TWeakPtr.h                 # Breaking circular refs
│   ├── 05_TUniquePtr.h               # Exclusive ownership
│   └── 06_RealWorld_Combined.h       # Complete game system
└── Exercises/
    ├── Exercise01_BasicPointers.h        # Fundamentals practice
    ├── Exercise01_BasicPointers_Solution.h
    ├── Exercise02_RealWorld.h            # Quest/inventory system
    └── Exercise02_RealWorld_Solution.h

Module02_TaskSystem/
├── README.md                          # Complete guide to async tasks
├── Examples/
│   ├── 01_BasicTasks.h               # Task fundamentals
│   ├── 02_TaskDependencies.h         # Task graphs & dependencies
│   ├── 03_GameThreadInteraction.h    # Safe UObject access
│   └── 04_ParallelPatterns.h         # Parallel algorithms
└── Exercises/
    ├── Exercise01_BasicAsync.h           # Async challenges
    └── Exercise01_BasicAsync_Solution.h

CLAUDE.md                              # Guidance for Claude Code AI assistant
README.md                              # This file
```

## How to Use This Training Program

### For Beginners:
1. Start with **Module 1** - Essential foundation for all Unreal C++ work
2. Read the README.md thoroughly
3. Study each example file in order
4. Attempt exercises without looking at solutions
5. Compare your solutions and learn from differences
6. Move to **Module 2** once comfortable with smart pointers

### For Intermediate Developers:
1. Skim Module 1 README for gaps in knowledge
2. Focus on examples that demonstrate unfamiliar patterns
3. Jump straight to Module 2 for async/parallel programming
4. Complete exercises to validate understanding

### For Reference:
- Use README decision matrices for quick lookups
- Refer to Quick Reference Cards at bottom of each README
- Study real-world examples (06_RealWorld_Combined.h) for patterns
- Review common pitfalls sections before code reviews

## Learning Objectives

### After Module 1, you will be able to:
- Choose the appropriate pointer type for any scenario
- Avoid common memory management pitfalls
- Implement proper UObject lifetime management
- Break circular references with weak pointers
- Apply RAII patterns with unique pointers
- Build complex systems with mixed pointer types

### After Module 2, you will be able to:
- Launch background tasks safely
- Build complex task dependency graphs
- Safely interact with UObjects from background threads
- Implement common parallel patterns
- Optimize task granularity for performance
- Profile and debug async code

## Key Principles

### Module 1 - Smart Pointers:
- ✅ **Always** use UPROPERTY() for UObject pointers
- ✅ Use TSharedPtr for shared non-UObject data
- ✅ Use TSharedRef when null is invalid
- ✅ Use TWeakPtr to break circular references
- ✅ Use TUniquePtr for exclusive ownership

### Module 2 - Task System:
- ⚠️ **NEVER** access UObjects from background threads
- ✅ Always use weak pointers when capturing UObject references
- ✅ Return to game thread for UObject access
- ✅ Batch work appropriately (1-10ms per task)
- ✅ Use Prerequisites for task dependencies

## Integration Between Modules

The modules work together - many async patterns use smart pointers:

```cpp
// Example: Combining both modules
TSharedPtr<FGameData> SharedData = MakeShared<FGameData>();

UE::Tasks::Launch(TEXT("Process"), [SharedData]()  // Capture by value (ref counted)
{
    // Background: Process shared data
    ProcessData(*SharedData);

    // Return to game thread
    AsyncTask(ENamedThreads::GameThread, [SharedData]()
    {
        // Apply to UObjects
        ApplyToGame(*SharedData);
    });
});
```

## Testing Your Knowledge

### Self-Assessment Questions:

**Module 1:**
- When should you use TSharedPtr vs TSharedRef?
- Why do circular TSharedPtr references cause memory leaks?
- When is it safe to use raw UObject pointers?
- What happens to a UObject* without UPROPERTY()?

**Module 2:**
- Why can't you access UObjects from background threads?
- How do you safely return results to the game thread?
- What's the optimal task granularity and why?
- When should you use Prerequisites vs nested tasks?

## Additional Resources

- **Unreal Engine Documentation** - Official API reference
- **Unreal Source Code** - Best examples are in engine source
- **Unreal Insights** - Profiler for task performance
- **Rider/Visual Studio** - Debugging async code

## Getting Help

If you encounter issues or have questions:
1. Review the relevant README sections
2. Study the example code comments
3. Check the solution files for pattern guidance
4. Consult Unreal Engine documentation
5. Search Unreal source code for real-world usage

## Next Training Modules (Future)

Potential future additions:
- Module 3: Delegates and Events
- Module 4: Reflection System and UProperties
- Module 5: Networking and Replication
- Module 6: Plugin Architecture

## Contributing

This is a personal training repository, but suggestions for improvements are welcome!

---

**Start your journey:** Open `Module01_SmartPointers/README.md` and begin learning!
