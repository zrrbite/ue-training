// Example 1: Basic Task Usage
// Demonstrates fundamental task launching and waiting

#pragma once

#include "CoreMinimal.h"
#include "Tasks/Task.h"

class FBasicTaskExamples
{
public:
	// Example 1: Simple background task
	void SimplestTask()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Simplest Task ==="));

		// Launch a task that runs in the background
		UE::Tasks::FTask Task = UE::Tasks::Launch(TEXT("SimpleTask"), []()
		{
			UE_LOG(LogTemp, Log, TEXT("Task running on background thread"));

			// Simulate work
			FPlatformProcess::Sleep(0.1f);

			UE_LOG(LogTemp, Log, TEXT("Task completed"));
		});

		UE_LOG(LogTemp, Log, TEXT("Task launched, doing other work..."));

		// Wait for completion
		Task.Wait();

		UE_LOG(LogTemp, Log, TEXT("Task finished, continuing"));
	}

	// Example 2: Task with return value
	void TaskWithReturnValue()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Task With Return Value ==="));

		auto Task = UE::Tasks::Launch(TEXT("Calculate"), []() -> int32
		{
			UE_LOG(LogTemp, Log, TEXT("Calculating..."));

			int32 Result = 0;
			for (int32 i = 1; i <= 100; i++)
			{
				Result += i;
			}

			return Result;  // Sum of 1 to 100
		});

		UE_LOG(LogTemp, Log, TEXT("Waiting for calculation..."));

		// Get the result (blocks until complete)
		int32 Result = Task.GetResult();

		UE_LOG(LogTemp, Log, TEXT("Result: %d"), Result);  // 5050
	}

	// Example 3: Check completion without blocking
	void CheckCompletionNonBlocking()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Non-Blocking Completion Check ==="));

		auto Task = UE::Tasks::Launch(TEXT("LongTask"), []()
		{
			FPlatformProcess::Sleep(2.0f);
			return 42;
		});

		// Check periodically without blocking
		while (!Task.IsCompleted())
		{
			UE_LOG(LogTemp, Log, TEXT("Task still running, doing other work..."));
			FPlatformProcess::Sleep(0.5f);
		}

		UE_LOG(LogTemp, Log, TEXT("Task completed! Result: %d"), Task.GetResult());
	}

	// Example 4: Multiple tasks
	void MultipleTasks()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Multiple Independent Tasks ==="));

		auto Task1 = UE::Tasks::Launch(TEXT("Task1"), []()
		{
			UE_LOG(LogTemp, Log, TEXT("Task1 starting"));
			FPlatformProcess::Sleep(0.1f);
			UE_LOG(LogTemp, Log, TEXT("Task1 done"));
			return 1;
		});

		auto Task2 = UE::Tasks::Launch(TEXT("Task2"), []()
		{
			UE_LOG(LogTemp, Log, TEXT("Task2 starting"));
			FPlatformProcess::Sleep(0.15f);
			UE_LOG(LogTemp, Log, TEXT("Task2 done"));
			return 2;
		});

		auto Task3 = UE::Tasks::Launch(TEXT("Task3"), []()
		{
			UE_LOG(LogTemp, Log, TEXT("Task3 starting"));
			FPlatformProcess::Sleep(0.05f);
			UE_LOG(LogTemp, Log, TEXT("Task3 done"));
			return 3;
		});

		// Wait for all tasks
		UE::Tasks::Wait(TArray<UE::Tasks::FTask>{Task1, Task2, Task3});

		UE_LOG(LogTemp, Log, TEXT("All tasks complete: %d, %d, %d"),
			Task1.GetResult(), Task2.GetResult(), Task3.GetResult());
	}

	// Example 5: Task priorities
	void TaskPriorities()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Task Priorities ==="));

		// Low priority task
		auto LowTask = UE::Tasks::Launch(
			TEXT("LowPriority"),
			[]()
			{
				UE_LOG(LogTemp, Log, TEXT("Low priority task running"));
				FPlatformProcess::Sleep(0.1f);
			},
			UE::Tasks::ETaskPriority::BackgroundLow
		);

		// Normal priority task
		auto NormalTask = UE::Tasks::Launch(
			TEXT("NormalPriority"),
			[]()
			{
				UE_LOG(LogTemp, Log, TEXT("Normal priority task running"));
				FPlatformProcess::Sleep(0.1f);
			},
			UE::Tasks::ETaskPriority::Normal
		);

		// High priority task
		auto HighTask = UE::Tasks::Launch(
			TEXT("HighPriority"),
			[]()
			{
				UE_LOG(LogTemp, Log, TEXT("High priority task running"));
				FPlatformProcess::Sleep(0.1f);
			},
			UE::Tasks::ETaskPriority::High
		);

		// High priority task likely runs first, but not guaranteed
		UE::Tasks::Wait(TArray{LowTask, NormalTask, HighTask});
	}

	// Example 6: Capturing variables
	void CapturingVariables()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Capturing Variables ==="));

		int32 LocalValue = 100;
		FString LocalString = TEXT("Hello from main thread");

		// Capture by value (safe - makes a copy)
		auto Task1 = UE::Tasks::Launch(TEXT("CaptureByValue"), [LocalValue, LocalString]()
		{
			UE_LOG(LogTemp, Log, TEXT("Captured value: %d"), LocalValue);
			UE_LOG(LogTemp, Log, TEXT("Captured string: %s"), *LocalString);
		});

		// Capture by reference (DANGEROUS - must ensure lifetime!)
		TArray<int32> Numbers = {1, 2, 3, 4, 5};

		auto Task2 = UE::Tasks::Launch(TEXT("CaptureByRef"), [&Numbers]()
		{
			int32 Sum = 0;
			for (int32 Num : Numbers)
			{
				Sum += Num;
			}
			UE_LOG(LogTemp, Log, TEXT("Sum of numbers: %d"), Sum);
		});

		// Wait before Numbers goes out of scope!
		Task2.Wait();

		Task1.Wait();
	}

	// Example 7: Nested tasks
	void NestedTasks()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Nested Tasks ==="));

		auto OuterTask = UE::Tasks::Launch(TEXT("OuterTask"), []()
		{
			UE_LOG(LogTemp, Log, TEXT("Outer task started"));

			// Launch inner task
			auto InnerTask = UE::Tasks::Launch(TEXT("InnerTask"), []()
			{
				UE_LOG(LogTemp, Log, TEXT("Inner task running"));
				FPlatformProcess::Sleep(0.1f);
				return 42;
			});

			// Wait for inner task
			int32 Result = InnerTask.GetResult();

			UE_LOG(LogTemp, Log, TEXT("Outer task got result: %d"), Result);

			return Result * 2;
		});

		int32 FinalResult = OuterTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("Final result: %d"), FinalResult);  // 84
	}
};
