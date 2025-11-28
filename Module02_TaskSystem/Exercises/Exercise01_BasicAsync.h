// Exercise 1: Basic Async Operations
// Implement async operations using Tasks::FTask

#pragma once

#include "CoreMinimal.h"
#include "Tasks/Task.h"

/*
 * EXERCISE 1: Implement Parallel Sum
 *
 * Calculate the sum of an array using multiple parallel tasks.
 * Divide the array into chunks and sum each chunk in parallel.
 */

class FExercise01_ParallelSum
{
public:
	int32 CalculateParallelSum(const TArray<int32>& Numbers)
	{
		// TODO: Implement parallel sum
		// 1. Divide Numbers into 4 chunks
		// 2. Launch 4 tasks to sum each chunk
		// 3. Wait for all tasks to complete
		// 4. Sum the results from all tasks

		// Hint: Use UE::Tasks::Launch and UE::Tasks::Wait

		return 0;  // Replace with actual implementation
	}
};

/*
 * EXERCISE 2: Async File Processing
 *
 * Simulate loading multiple files asynchronously and combining results.
 */

class FExercise02_AsyncFileProcessing
{
public:
	struct FFileData
	{
		FString FileName;
		TArray<FString> Lines;
		int32 LineCount;
	};

	void ProcessFilesAsync(const TArray<FString>& FilePaths)
	{
		// TODO: Implement async file processing
		// 1. Launch a task for each file
		// 2. Each task should "load" the file (simulate with Sleep)
		// 3. Parse the file data
		// 4. Wait for all files to be processed
		// 5. Combine results and print statistics

		// Hint: Store tasks in TArray and use UE::Tasks::Wait
	}

private:
	FFileData SimulateLoadFile(const FString& FilePath)
	{
		// Simulate file loading
		FPlatformProcess::Sleep(0.1f);

		FFileData Data;
		Data.FileName = FilePath;
		Data.LineCount = FMath::RandRange(10, 100);

		return Data;
	}
};

/*
 * EXERCISE 3: Task Dependencies
 *
 * Create a task pipeline with dependencies.
 */

class FExercise03_TaskPipeline
{
public:
	void RunPipeline()
	{
		// TODO: Create a pipeline with the following stages:
		// 1. LoadData: Returns TArray<int32> with 100 random numbers
		// 2. FilterData: Takes LoadData output, filters even numbers
		// 3. TransformData: Takes FilterData output, squares each number
		// 4. AggregateData: Takes TransformData output, sums all numbers
		//
		// Each stage should depend on the previous stage
		// Print the final result

		// Hint: Use Prerequisites to create dependencies
	}
};

/*
 * EXERCISE 4: Game Thread Safety
 *
 * Fix the unsafe UObject access pattern.
 */

UCLASS()
class UExercise04_Component : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 ProcessedCount = 0;

	void ProcessDataUnsafe()
	{
		// TODO: This code has a bug - fix it!
		// The task accesses a UObject member from a background thread

		UE::Tasks::Launch(TEXT("UnsafeTask"), [this]()
		{
			// Process data
			for (int32 i = 0; i < 100; i++)
			{
				// Heavy computation
				FMath::Sin(static_cast<float>(i));
			}

			// BUG: Accessing UObject member from background thread!
			ProcessedCount = 100;
		});

		// TODO: Fix the code above to safely update ProcessedCount
		// Hint: Use AsyncTask(ENamedThreads::GameThread, ...)
	}
};

/*
 * EXERCISE 5: Optimal Batching
 *
 * Implement parallel processing with optimal batch size.
 */

class FExercise05_OptimalBatching
{
public:
	TArray<float> ProcessLargeDataset(const TArray<float>& Input)
	{
		// TODO: Process a large dataset in parallel
		// 1. Calculate optimal batch size based on number of cores
		// 2. Create tasks to process each batch
		// 3. Each task should: square the value, take sine, take square root
		// 4. Merge results from all batches
		//
		// Requirements:
		// - Use all available cores efficiently
		// - Batch size should be at least 1000 items
		// - Avoid creating too many small tasks

		TArray<float> Results;
		// TODO: Implement

		return Results;
	}

private:
	float ComplexCalculation(float Value)
	{
		return FMath::Sqrt(FMath::Sin(Value * Value));
	}
};

// See Exercise01_BasicAsync_Solution.h for answers
