# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository is an Unreal Engine C++ training program with comprehensive modules covering:
- **Module 1:** Smart Pointers and References (UObject, TSharedPtr, TSharedRef, TWeakPtr, TUniquePtr)
- **Module 2:** Task System (Tasks::FTask, AsyncTask, parallel patterns)

Each module contains theory (README.md), practical examples, and exercises with solutions.

## Repository Structure

```
Module01_SmartPointers/
├── README.md              # Comprehensive smart pointer guide
├── Examples/              # 6 example files (01-06)
└── Exercises/             # 2 exercises with solutions

Module02_TaskSystem/
├── README.md              # Comprehensive task system guide
├── Examples/              # 4 example files (01-04)
└── Exercises/             # Exercises with solutions

README.md                  # Main training program overview
CLAUDE.md                  # This file
```

## Unreal Engine C++ Guidelines

### Coding Standards
- Follow Unreal Engine coding standards (PascalCase for classes/functions)
- Prefix classes: U (UObject), A (AActor), F (structs), E (enums), I (interfaces)
- Use Unreal's reflection system (UCLASS, UPROPERTY, UFUNCTION) appropriately
- Include GENERATED_BODY() in reflected classes

### Smart Pointer Usage (Module 1)
- **UObject pointers:** Always use UPROPERTY() for strong references
- **TSharedPtr:** Shared ownership of non-UObject data
- **TSharedRef:** Shared ownership when null is invalid
- **TWeakPtr:** Breaking circular references, optional references
- **TUniquePtr:** Exclusive ownership, RAII patterns
- **Never** use TSharedPtr/TUniquePtr with UObjects (use Unreal's GC instead)

### Task System Usage (Module 2)
- **CRITICAL:** Never access UObjects from background threads
- Use `UE::Tasks::Launch` for modern UE5 async operations
- Use `AsyncTask(ENamedThreads::GameThread, ...)` to return to game thread
- Always use TWeakObjectPtr when capturing UObject references in tasks
- Batch work appropriately (1-10ms per task for optimal performance)
- Use Prerequisites for task dependencies

### Container Types
- Prefer Unreal containers: TArray, TMap, TSet, TQueue
- Use TArray instead of std::vector
- Use TMap instead of std::map
- Use FString instead of std::string

### Memory Management
- UObjects managed by garbage collector - never manually delete
- Non-UObjects use smart pointers or RAII
- Be mindful of lifetime when capturing variables in async tasks

## Training Module Guidelines

### When Adding New Examples
- Follow naming convention: `##_DescriptiveName.h`
- Include comprehensive comments explaining the pattern
- Demonstrate both correct and incorrect usage (marked clearly)
- Add logging for visibility during execution
- Keep examples focused on single concepts

### When Adding New Exercises
- Provide clear TODOs for what needs to be implemented
- Include hints without giving away the solution
- Create corresponding `_Solution.h` file with complete implementation
- Add explanatory comments in solutions
- Include test/verification functions

### Documentation Standards
- Keep READMEs comprehensive but scannable
- Include decision matrices for quick reference
- Provide "Quick Reference Card" sections
- Add "Common Pitfalls" sections with examples
- Link to official Unreal documentation

## Development Workflow

This is a training repository - no build commands needed. Code examples are designed to be:
1. Studied and understood
2. Copied into actual Unreal projects for testing
3. Modified and experimented with

## Common Patterns in This Repository

### Example File Pattern
```cpp
// Example N: Title
// Brief description

#pragma once
#include "CoreMinimal.h"

class FExampleClass
{
public:
    void ExampleMethod()
    {
        // Demonstration code with comments
    }
};
```

### Exercise File Pattern
```cpp
// Exercise N: Title
// Instructions

#pragma once

// TODO: Implement...
// Hint: ...

// See ExerciseN_Solution.h for answers
```

## Key Learning Points to Emphasize

### Module 1 (Smart Pointers)
- Decision matrix: when to use which pointer type
- Circular reference detection and prevention
- UObject vs non-UObject memory management
- Thread safety implications (expanded in Module 2)

### Module 2 (Task System)
- Thread safety is CRITICAL
- Game thread vs background thread distinction
- Task granularity optimization
- Common parallel patterns (map-reduce, producer-consumer, etc.)

## Integration Between Modules

Many examples combine concepts from both modules:
- Using TSharedPtr to safely share data between tasks
- Using TWeakPtr for optional references in async callbacks
- Capturing smart pointers in task lambdas
- Combining task dependencies with shared ownership

## Future Expansion

When adding new modules, follow this structure:
1. Create `ModuleXX_TopicName/` directory
2. Add comprehensive README.md
3. Create Examples/ subdirectory with numbered examples
4. Create Exercises/ subdirectory with exercises and solutions
5. Update main README.md with module status and overview
6. Update this CLAUDE.md with module-specific guidelines
