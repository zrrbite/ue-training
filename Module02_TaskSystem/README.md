# Module 2: Task System (Tasks::FTask and AsyncTask)

## Overview

Unreal Engine provides powerful task systems for asynchronous and parallel execution. This module covers `Tasks::FTask` (UE5), legacy `AsyncTask`, the Task Graph System, and when to use each approach.

## Table of Contents

1. [Introduction to Async Programming in Unreal](#introduction)
2. [Tasks::FTask (UE5+)](#tasksfTask)
3. [Legacy AsyncTask System](#legacy-asynctask)
4. [Task Dependencies and Chaining](#task-dependencies)
5. [Named Threads vs Task Graph](#named-threads-vs-task-graph)
6. [Performance Considerations](#performance-considerations)
7. [Common Patterns](#common-patterns)
8. [Pitfalls and Best Practices](#pitfalls-and-best-practices)

---

## Introduction to Async Programming in Unreal

### Why Use Tasks?

- **Prevent frame drops** - Move expensive work off the game thread
- **Utilize multiple cores** - Modern CPUs have many cores sitting idle
- **Improve responsiveness** - Keep UI and gameplay smooth
- **Scale workloads** - Process large datasets in parallel

### Thread Safety Rules

⚠️ **Critical**: Most Unreal APIs are **NOT thread-safe**

**Game Thread Only:**
- UObject access (reading/writing properties)
- Actor spawning and destruction
- Component modification
- Blueprint execution
- Most engine subsystems

**Thread-Safe:**
- Pure math operations
- Data structure manipulation (with proper synchronization)
- File I/O (with care)
- Custom thread-safe systems

### Unreal's Threading Model

```
Game Thread (Main)     - Gameplay logic, UObjects, rendering commands
Render Thread          - Rendering preparation
RHI Thread             - GPU command submission
Task Graph Workers     - Parallel task execution
Named Threads          - Dedicated threads (Async Loading, Audio, etc.)
Thread Pool            - Generic worker threads
```

---

## Tasks::FTask (UE5+)

### Overview

`Tasks::FTask` is the modern task system introduced in UE5. It provides a cleaner, more powerful API compared to legacy systems.

### Key Features

- Modern C++ syntax with lambdas
- Built-in task dependencies (prerequisites)
- Efficient task graph scheduling
- Better debugging support
- Integration with profiling tools

### Basic Usage

```cpp
#include "Tasks/Task.h"

// Launch a simple task
UE::Tasks::FTask Task = UE::Tasks::Launch(TEXT("MyTask"), []()
{
    // This runs on a background thread
    // DO NOT access UObjects here!

    int32 Result = ExpensiveCalculation();
    return Result;
});

// Wait for completion (blocks calling thread)
Task.Wait();

// Check if completed without blocking
if (Task.IsCompleted())
{
    // Task is done
}
```

### Task Priorities

```cpp
// Different priority levels
UE::Tasks::ETaskPriority::High       // Process ASAP
UE::Tasks::ETaskPriority::Normal     // Default
UE::Tasks::ETaskPriority::Low        // Process when idle
UE::Tasks::ETaskPriority::BackgroundHigh
UE::Tasks::ETaskPriority::BackgroundNormal
UE::Tasks::ETaskPriority::BackgroundLow
```

### Launching Tasks

```cpp
// Basic launch
auto Task = UE::Tasks::Launch(TEXT("TaskName"), []()
{
    // Work here
});

// Launch with priority
auto PriorityTask = UE::Tasks::Launch(
    TEXT("HighPriorityTask"),
    []() { /* Work */ },
    UE::Tasks::ETaskPriority::High
);

// Launch with prerequisites (dependencies)
auto Task1 = UE::Tasks::Launch(TEXT("Task1"), []() { /* Work 1 */ });
auto Task2 = UE::Tasks::Launch(TEXT("Task2"), []() { /* Work 2 */ });

// Task3 runs after Task1 and Task2 complete
auto Task3 = UE::Tasks::Launch(
    TEXT("Task3"),
    []() { /* Work 3 - runs after Task1 and Task2 */ },
    UE::Tasks::ETaskPriority::Normal,
    UE::Tasks::Prerequisites(Task1, Task2)
);
```

### Returning to Game Thread

```cpp
void ProcessDataAsync()
{
    // Capture UObject pointer (be careful!)
    AActor* ActorPtr = GetOwner();

    UE::Tasks::Launch(TEXT("ProcessData"), [ActorPtr]()
    {
        // Background thread - NO UObject access!
        TArray<int32> Results = HeavyComputation();

        // Return to game thread for UObject access
        AsyncTask(ENamedThreads::GameThread, [ActorPtr, Results]()
        {
            // NOW on game thread - safe to access UObjects
            if (IsValid(ActorPtr))
            {
                // Use results with actor
                ActorPtr->OnComputationComplete(Results);
            }
        });
    });
}
```

---

## Legacy AsyncTask System

### Overview

The legacy `AsyncTask` function predates `Tasks::FTask`. It's still widely used and supported.

### Basic Usage

```cpp
// Run on background thread
AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, []()
{
    // Background work here
    ExpensiveOperation();
});

// Run on game thread
AsyncTask(ENamedThreads::GameThread, []()
{
    // Game thread work - safe for UObjects
    UpdateUIElements();
});

// Run on specific thread
AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, []()
{
    // Any normal priority thread
});
```

### Common Thread Types

```cpp
ENamedThreads::GameThread                      // Main game thread
ENamedThreads::AnyBackgroundThreadNormalTask   // Any background worker
ENamedThreads::AnyBackgroundHiPriTask          // High priority background
ENamedThreads::AnyThread                       // Any available thread
ENamedThreads::AnyNormalThreadNormalTask       // Any normal thread
```

### Pattern: Async Processing with Game Thread Callback

```cpp
void LoadAndProcessData()
{
    // Step 1: Load data on background thread
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
    {
        // Load data (thread-safe operation)
        TArray<uint8> RawData = LoadFileFromDisk();

        // Process data (no UObject access)
        TArray<FProcessedData> ProcessedData = ProcessRawData(RawData);

        // Step 2: Return to game thread to apply results
        AsyncTask(ENamedThreads::GameThread, [this, ProcessedData]()
        {
            // NOW safe to access UObjects
            ApplyProcessedData(ProcessedData);
            OnLoadComplete.Broadcast();
        });
    });
}
```

---

## Task Dependencies and Chaining

### Why Dependencies Matter

- Ensure correct execution order
- Manage complex workflows
- Avoid race conditions
- Create efficient pipelines

### Using Prerequisites

```cpp
// Create a pipeline
auto LoadTask = UE::Tasks::Launch(TEXT("Load"), []()
{
    return LoadDataFromDisk();
});

auto ProcessTask = UE::Tasks::Launch(
    TEXT("Process"),
    [LoadTask]()
    {
        auto Data = LoadTask.GetResult();  // Get result from dependency
        return ProcessData(Data);
    },
    UE::Tasks::Prerequisites(LoadTask)  // Depends on LoadTask
);

auto SaveTask = UE::Tasks::Launch(
    TEXT("Save"),
    [ProcessTask]()
    {
        auto Processed = ProcessTask.GetResult();
        SaveToDisk(Processed);
    },
    UE::Tasks::Prerequisites(ProcessTask)  // Depends on ProcessTask
);

// Wait for entire pipeline
SaveTask.Wait();
```

### Multiple Dependencies

```cpp
// Fan-out pattern
auto SourceTask = UE::Tasks::Launch(TEXT("Source"), []() { return GetData(); });

auto Worker1 = UE::Tasks::Launch(
    TEXT("Worker1"),
    [SourceTask]() { return Process1(SourceTask.GetResult()); },
    UE::Tasks::Prerequisites(SourceTask)
);

auto Worker2 = UE::Tasks::Launch(
    TEXT("Worker2"),
    [SourceTask]() { return Process2(SourceTask.GetResult()); },
    UE::Tasks::Prerequisites(SourceTask)
);

auto Worker3 = UE::Tasks::Launch(
    TEXT("Worker3"),
    [SourceTask]() { return Process3(SourceTask.GetResult()); },
    UE::Tasks::Prerequisites(SourceTask)
);

// Fan-in pattern - combine results
auto CombineTask = UE::Tasks::Launch(
    TEXT("Combine"),
    [Worker1, Worker2, Worker3]()
    {
        return Combine(
            Worker1.GetResult(),
            Worker2.GetResult(),
            Worker3.GetResult()
        );
    },
    UE::Tasks::Prerequisites(Worker1, Worker2, Worker3)
);
```

---

## Named Threads vs Task Graph

### Named Threads

**When to Use:**
- Need guaranteed dedicated thread
- Continuous background work
- Specific thread affinity required
- Legacy system integration

**Examples:**
- `GameThread` - Main gameplay and UObject access
- `RenderThread` - Rendering commands
- `AudioThread` - Audio processing
- `AsyncLoadingThread` - Asset streaming

### Task Graph

**When to Use:**
- Short-lived parallel work
- Variable workload
- Efficient multi-core utilization
- Don't need specific thread

**Advantages:**
- Automatic load balancing
- Better CPU utilization
- Less overhead than creating threads
- Built-in dependency management

### Decision Matrix

| Scenario | Use This |
|----------|----------|
| Quick background calculation | Task Graph |
| UObject modification | Game Thread |
| Parallel data processing | Task Graph with multiple tasks |
| Continuous monitoring/polling | Named Thread or Ticker |
| File I/O | Task Graph (AnyBackgroundThread) |
| Rendering commands | Render Thread |
| Need specific thread | Named Thread |
| Need task dependencies | Task Graph (Tasks::FTask) |

---

## Performance Considerations

### Task Granularity

**Too Fine-Grained:**
```cpp
// BAD: Task overhead exceeds work
for (int32 i = 0; i < 1000000; i++)
{
    UE::Tasks::Launch(TEXT("TinyTask"), [i]()
    {
        SimpleCalculation(i);  // Too small!
    });
}
```

**Appropriate Granularity:**
```cpp
// GOOD: Batch work into reasonable chunks
const int32 BatchSize = 10000;
const int32 NumTasks = Items.Num() / BatchSize;

TArray<UE::Tasks::FTask> Tasks;
for (int32 TaskIndex = 0; TaskIndex < NumTasks; TaskIndex++)
{
    Tasks.Add(UE::Tasks::Launch(TEXT("BatchTask"), [TaskIndex, BatchSize, &Items]()
    {
        int32 Start = TaskIndex * BatchSize;
        int32 End = FMath::Min(Start + BatchSize, Items.Num());

        for (int32 i = Start; i < End; i++)
        {
            ProcessItem(Items[i]);
        }
    }));
}

// Wait for all batches
UE::Tasks::Wait(Tasks);
```

### Rule of Thumb

- **Minimum task duration**: 0.1ms - 1ms
- **Ideal task duration**: 1ms - 10ms
- **Tasks too short**: Overhead dominates
- **Tasks too long**: Poor load balancing

### Memory Considerations

```cpp
// BAD: Capturing large data by value
TArray<FLargeStruct> HugeArray;  // 100MB
UE::Tasks::Launch(TEXT("Bad"), [HugeArray]()  // Copies 100MB!
{
    Process(HugeArray);
});

// GOOD: Capture by reference (with care!)
UE::Tasks::Launch(TEXT("Good"), [&HugeArray]()
{
    Process(HugeArray);  // No copy, but ensure lifetime!
});

// BETTER: Use TSharedPtr for safety
TSharedPtr<TArray<FLargeStruct>> SharedArray =
    MakeShared<TArray<FLargeStruct>>(HugeArray);

UE::Tasks::Launch(TEXT("Best"), [SharedArray]()
{
    Process(*SharedArray);  // Shared ownership, safe lifetime
});
```

---

## Common Patterns

### Pattern 1: Parallel For-Each

```cpp
void ProcessItemsInParallel(const TArray<FItem>& Items)
{
    const int32 NumThreads = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
    const int32 ItemsPerTask = FMath::Max(1, Items.Num() / NumThreads);

    TArray<UE::Tasks::FTask> Tasks;

    for (int32 TaskIdx = 0; TaskIdx < NumThreads; TaskIdx++)
    {
        const int32 StartIdx = TaskIdx * ItemsPerTask;
        const int32 EndIdx = (TaskIdx == NumThreads - 1)
            ? Items.Num()
            : StartIdx + ItemsPerTask;

        if (StartIdx < Items.Num())
        {
            Tasks.Add(UE::Tasks::Launch(TEXT("ProcessBatch"), [&Items, StartIdx, EndIdx]()
            {
                for (int32 i = StartIdx; i < EndIdx; i++)
                {
                    ProcessItem(Items[i]);
                }
            }));
        }
    }

    UE::Tasks::Wait(Tasks);
}
```

### Pattern 2: Producer-Consumer

```cpp
class FAsyncProducerConsumer
{
private:
    TQueue<FWorkItem, EQueueMode::Mpsc> WorkQueue;  // Multi-producer, single-consumer
    std::atomic<bool> bRunning{true};

public:
    void Start()
    {
        // Consumer task
        UE::Tasks::Launch(TEXT("Consumer"), [this]()
        {
            while (bRunning)
            {
                FWorkItem Item;
                if (WorkQueue.Dequeue(Item))
                {
                    ProcessItem(Item);
                }
                else
                {
                    FPlatformProcess::Sleep(0.001f);  // Brief sleep if empty
                }
            }
        });
    }

    void AddWork(const FWorkItem& Item)
    {
        WorkQueue.Enqueue(Item);  // Thread-safe enqueue
    }

    void Stop()
    {
        bRunning = false;
    }
};
```

### Pattern 3: Async Load and Apply

```cpp
UCLASS()
class UAsyncDataLoader : public UObject
{
    GENERATED_BODY()

public:
    void LoadDataAsync(const FString& FilePath)
    {
        // Launch background task
        UE::Tasks::Launch(TEXT("LoadData"), [this, FilePath]()
        {
            // Background: Load file (thread-safe)
            TArray<uint8> FileData;
            FFileHelper::LoadFileToArray(FileData, *FilePath);

            // Background: Parse data (no UObject access)
            TArray<FParsedData> ParsedData = ParseFileData(FileData);

            // Return to game thread for UObject access
            AsyncTask(ENamedThreads::GameThread, [this, ParsedData]()
            {
                // Game thread: Safe to access UObjects
                if (IsValid(this))
                {
                    ApplyParsedData(ParsedData);
                    OnLoadCompleted.Broadcast();
                }
            });
        });
    }

private:
    UPROPERTY(BlueprintAssignable)
    FOnLoadCompleted OnLoadCompleted;
};
```

---

## Pitfalls and Best Practices

### ❌ DON'T: Access UObjects from Background Threads

```cpp
// WRONG - Will crash!
UE::Tasks::Launch(TEXT("BadTask"), [this]()
{
    MyActor->SetActorLocation(FVector::ZeroVector);  // CRASH!
});
```

### ✅ DO: Use Game Thread for UObject Access

```cpp
// CORRECT
UE::Tasks::Launch(TEXT("GoodTask"), [this]()
{
    FVector NewLocation = CalculatePosition();  // Background work

    AsyncTask(ENamedThreads::GameThread, [this, NewLocation]()
    {
        MyActor->SetActorLocation(NewLocation);  // Safe on game thread
    });
});
```

### ❌ DON'T: Capture 'this' Without Lifetime Guarantee

```cpp
// DANGEROUS - 'this' might be destroyed
void UMyComponent::StartAsyncWork()
{
    UE::Tasks::Launch(TEXT("Risky"), [this]()
    {
        // If component is destroyed, 'this' is dangling!
        Process();  // CRASH!
    });
}
```

### ✅ DO: Use Weak Pointers or Ensure Lifetime

```cpp
// SAFE - Check validity
void UMyComponent::StartAsyncWork()
{
    TWeakObjectPtr<UMyComponent> WeakThis(this);

    UE::Tasks::Launch(TEXT("Safe"), [WeakThis]()
    {
        TArray<int32> Results = BackgroundWork();

        AsyncTask(ENamedThreads::GameThread, [WeakThis, Results]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->ApplyResults(Results);
            }
        });
    });
}
```

### ❌ DON'T: Create Too Many Tiny Tasks

```cpp
// BAD - Massive overhead
for (const FItem& Item : Items)  // 1 million items
{
    UE::Tasks::Launch(TEXT("Process"), [Item]()
    {
        SmallOperation(Item);  // 1 microsecond of work
    });
}
```

### ✅ DO: Batch Work Appropriately

```cpp
// GOOD - Batched tasks
const int32 BatchSize = 1000;
for (int32 i = 0; i < Items.Num(); i += BatchSize)
{
    UE::Tasks::Launch(TEXT("ProcessBatch"), [i, BatchSize, &Items]()
    {
        int32 End = FMath::Min(i + BatchSize, Items.Num());
        for (int32 j = i; j < End; j++)
        {
            SmallOperation(Items[j]);
        }
    });
}
```

### ❌ DON'T: Forget Thread Safety for Shared Data

```cpp
// WRONG - Race condition!
TArray<int32> SharedArray;

for (int32 i = 0; i < 10; i++)
{
    UE::Tasks::Launch(TEXT("RacyTask"), [&SharedArray, i]()
    {
        SharedArray.Add(i);  // RACE CONDITION!
    });
}
```

### ✅ DO: Use Proper Synchronization

```cpp
// CORRECT - Using mutex
TArray<int32> SharedArray;
FCriticalSection Mutex;

for (int32 i = 0; i < 10; i++)
{
    UE::Tasks::Launch(TEXT("SafeTask"), [&SharedArray, &Mutex, i]()
    {
        FScopeLock Lock(&Mutex);
        SharedArray.Add(i);  // Thread-safe
    });
}

// OR BETTER - Avoid sharing, merge results later
TArray<UE::Tasks::FTask> Tasks;
TArray<TArray<int32>> PerTaskResults;
PerTaskResults.SetNum(10);

for (int32 i = 0; i < 10; i++)
{
    Tasks.Add(UE::Tasks::Launch(TEXT("IndependentTask"), [&PerTaskResults, i]()
    {
        PerTaskResults[i].Add(i);  // No sharing, no mutex needed
    }));
}

UE::Tasks::Wait(Tasks);

// Merge results on game thread
for (const TArray<int32>& Results : PerTaskResults)
{
    SharedArray.Append(Results);
}
```

---

## Best Practices Summary

### ✅ DO:
- Use `Tasks::FTask` for new UE5 code
- Batch work into appropriately-sized tasks (1-10ms)
- Return to game thread for UObject access
- Use weak pointers when capturing UObject references
- Use prerequisites for task dependencies
- Profile your async code
- Consider task granularity vs overhead

### ❌ DON'T:
- Access UObjects from background threads
- Create millions of tiny tasks
- Capture raw UObject pointers without lifetime checks
- Forget about thread safety for shared data
- Block the game thread with `Wait()` unless necessary
- Use AsyncTask for complex dependencies (use Tasks::FTask instead)

---

## When to Use What

| Use Case | Recommended Approach |
|----------|---------------------|
| Simple background calculation | `UE::Tasks::Launch` |
| Complex task dependencies | `Tasks::FTask` with Prerequisites |
| UObject modification | `AsyncTask(ENamedThreads::GameThread)` |
| Parallel data processing | Multiple `Tasks::FTask` with `Wait()` |
| Background loading with game thread callback | Background task → Game thread AsyncTask |
| Legacy code maintenance | Continue using `AsyncTask` |
| Real-time continuous work | Ticker delegate or Named Thread |

---

## Examples Overview

The `Examples/` directory contains comprehensive demonstrations:

1. **01_BasicTasks.h** - Fundamental task operations
   - Simple tasks, return values, multiple tasks, priorities, capturing variables

2. **02_TaskDependencies.h** - Building task graphs
   - Simple chains, fan-out/fan-in patterns, diamond dependencies, pipelines

3. **03_GameThreadInteraction.h** - Safe UObject access
   - Background work with game thread callbacks, weak pointers, multi-stage processing

4. **04_ParallelPatterns.h** - Common parallel algorithms
   - Parallel for-each, map-reduce, producer-consumer, pipeline, optimal batching

## Exercises Overview

The `Exercises/` directory contains practical challenges:

1. **Exercise01_BasicAsync.h** - Fundamental async operations
   - Parallel sum, async file processing, task pipelines, game thread safety, optimal batching
   - **Exercise01_BasicAsync_Solution.h** - Complete solutions with explanations

## Learning Path

### Recommended Order:
1. Read through this README thoroughly, especially the thread safety rules
2. Study 01_BasicTasks.h - understand the fundamentals
3. Study 02_TaskDependencies.h - learn how to build task graphs
4. Study 03_GameThreadInteraction.h - **CRITICAL** for avoiding crashes
5. Study 04_ParallelPatterns.h - learn practical patterns
6. Attempt Exercise01 problems one by one
7. Compare your solutions with the provided solutions
8. Experiment in your own Unreal project
9. **Profile your async code** using Unreal Insights

### Critical Concepts to Master:
- **Thread safety rules** - Never access UObjects from background threads!
- Using weak pointers when capturing UObject references
- Returning to game thread with AsyncTask
- Task dependencies with Prerequisites
- Optimal task granularity (not too small, not too large)
- Batching work for efficiency

## Common Mistakes to Avoid

### ⚠️ CRITICAL - Will Crash:
```cpp
// WRONG - Accessing UObject from background thread
UE::Tasks::Launch(TEXT("Bad"), [this]() {
    MyActor->SetLocation(...);  // CRASH!
});

// CORRECT
UE::Tasks::Launch(TEXT("Good"), [this]() {
    FVector NewLoc = Calculate();
    AsyncTask(ENamedThreads::GameThread, [this, NewLoc]() {
        if (IsValid(this)) {
            MyActor->SetLocation(NewLoc);  // Safe
        }
    });
});
```

### ⚠️ Performance Issues:
```cpp
// WRONG - Too many tiny tasks
for (int i = 0; i < 1000000; i++) {
    Launch([i]() { TinyWork(i); });  // Huge overhead!
}

// CORRECT - Batched work
const int BatchSize = 10000;
for (int i = 0; i < Items.Num(); i += BatchSize) {
    Launch([i, BatchSize]() {
        for (int j = i; j < i + BatchSize; j++) {
            TinyWork(j);
        }
    });
}
```

## Next Steps

1. Work through all examples in order
2. Complete all exercises
3. Create async operations in your own Unreal project
4. **Profile with Unreal Insights** to verify performance
5. Review Module 1 if you need pointer refreshers
6. Build a real-world async system combining both modules

## Additional Resources

- [Unreal Engine Documentation: Task System](https://docs.unrealengine.com/5.0/en-US/tasks-systems-in-unreal-engine/)
- [Unreal Engine Documentation: Async](https://docs.unrealengine.com/5.0/en-US/asynchronous-asset-loading-in-unreal-engine/)
- [Unreal Insights Profiler](https://docs.unrealengine.com/5.0/en-US/unreal-insights-in-unreal-engine/)
- [Unreal Engine Source: Task.h](https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Core/Public/Tasks/Task.h)

## Quick Reference Card

```cpp
// Launch a task
UE::Tasks::FTask Task = UE::Tasks::Launch(
    TEXT("TaskName"),
    []() { /* Work */ },
    UE::Tasks::ETaskPriority::Normal  // Optional
);

// Wait for completion
Task.Wait();
TResult Result = Task.GetResult();

// Check without blocking
if (Task.IsCompleted()) { }

// Dependencies
auto Task2 = UE::Tasks::Launch(
    TEXT("Dependent"),
    []() { /* Work */ },
    UE::Tasks::Prerequisites(Task1)
);

// Multiple tasks
UE::Tasks::Wait(TArray{Task1, Task2, Task3});

// Game thread callback
UE::Tasks::Launch(TEXT("BG"), []() {
    auto Result = HeavyWork();
    AsyncTask(ENamedThreads::GameThread, [Result]() {
        // Safe for UObjects
    });
});

// Legacy AsyncTask
AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, []() {
    // Background work
});
AsyncTask(ENamedThreads::GameThread, []() {
    // Game thread work
});
```
