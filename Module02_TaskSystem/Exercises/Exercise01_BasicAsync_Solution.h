// Exercise 1: Solutions

#pragma once

#include "CoreMinimal.h"
#include "Tasks/Task.h"

/*
 * SOLUTION 1: Parallel Sum Implementation
 */

class FExercise01_ParallelSum_Solution
{
public:
	int32 CalculateParallelSum(const TArray<int32>& Numbers)
	{
		if (Numbers.Num() == 0)
		{
			return 0;
		}

		// Divide into 4 chunks
		const int32 NumChunks = 4;
		const int32 ChunkSize = Numbers.Num() / NumChunks;

		TArray<UE::Tasks::FTask> Tasks;

		// Launch tasks for each chunk
		for (int32 ChunkIdx = 0; ChunkIdx < NumChunks; ChunkIdx++)
		{
			const int32 StartIdx = ChunkIdx * ChunkSize;
			const int32 EndIdx = (ChunkIdx == NumChunks - 1)
				? Numbers.Num()
				: StartIdx + ChunkSize;

			Tasks.Add(UE::Tasks::Launch(
				TEXT("SumChunk"),
				[&Numbers, StartIdx, EndIdx]() -> int32
				{
					int32 ChunkSum = 0;
					for (int32 i = StartIdx; i < EndIdx; i++)
					{
						ChunkSum += Numbers[i];
					}

					UE_LOG(LogTemp, Log, TEXT("Chunk sum [%d to %d]: %d"), StartIdx, EndIdx, ChunkSum);
					return ChunkSum;
				}
			));
		}

		// Wait for all tasks
		UE::Tasks::Wait(Tasks);

		// Sum the results
		int32 TotalSum = 0;
		for (const UE::Tasks::FTask& Task : Tasks)
		{
			TotalSum += Task.GetResult();
		}

		UE_LOG(LogTemp, Log, TEXT("Total sum: %d"), TotalSum);
		return TotalSum;
	}
};

/*
 * SOLUTION 2: Async File Processing Implementation
 */

class FExercise02_AsyncFileProcessing_Solution
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
		UE_LOG(LogTemp, Log, TEXT("=== Processing %d files asynchronously ==="), FilePaths.Num());

		TArray<UE::Tasks::FTask> FileTasks;

		// Launch task for each file
		for (const FString& FilePath : FilePaths)
		{
			FileTasks.Add(UE::Tasks::Launch(
				TEXT("LoadFile"),
				[this, FilePath]() -> FFileData
				{
					UE_LOG(LogTemp, Log, TEXT("Loading file: %s"), *FilePath);
					return SimulateLoadFile(FilePath);
				}
			));
		}

		// Wait for all files
		UE::Tasks::Wait(FileTasks);

		// Combine results
		int32 TotalLines = 0;
		for (const UE::Tasks::FTask& Task : FileTasks)
		{
			FFileData Data = Task.GetResult();
			TotalLines += Data.LineCount;

			UE_LOG(LogTemp, Log, TEXT("File: %s, Lines: %d"), *Data.FileName, Data.LineCount);
		}

		UE_LOG(LogTemp, Log, TEXT("Total lines across all files: %d"), TotalLines);
	}

private:
	FFileData SimulateLoadFile(const FString& FilePath)
	{
		FPlatformProcess::Sleep(0.1f);

		FFileData Data;
		Data.FileName = FilePath;
		Data.LineCount = FMath::RandRange(10, 100);

		return Data;
	}
};

/*
 * SOLUTION 3: Task Pipeline Implementation
 */

class FExercise03_TaskPipeline_Solution
{
public:
	void RunPipeline()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Task Pipeline ==="));

		// Stage 1: Load data
		auto LoadTask = UE::Tasks::Launch(TEXT("LoadData"), []() -> TArray<int32>
		{
			UE_LOG(LogTemp, Log, TEXT("Stage 1: Loading data"));

			TArray<int32> Data;
			for (int32 i = 0; i < 100; i++)
			{
				Data.Add(FMath::RandRange(1, 100));
			}

			UE_LOG(LogTemp, Log, TEXT("Loaded %d numbers"), Data.Num());
			return Data;
		});

		// Stage 2: Filter even numbers
		auto FilterTask = UE::Tasks::Launch(
			TEXT("FilterData"),
			[LoadTask]() -> TArray<int32>
			{
				UE_LOG(LogTemp, Log, TEXT("Stage 2: Filtering data"));

				TArray<int32> InputData = LoadTask.GetResult();
				TArray<int32> FilteredData;

				for (int32 Value : InputData)
				{
					if (Value % 2 == 0)
					{
						FilteredData.Add(Value);
					}
				}

				UE_LOG(LogTemp, Log, TEXT("Filtered to %d even numbers"), FilteredData.Num());
				return FilteredData;
			},
			UE::Tasks::Prerequisites(LoadTask)
		);

		// Stage 3: Transform (square)
		auto TransformTask = UE::Tasks::Launch(
			TEXT("TransformData"),
			[FilterTask]() -> TArray<int32>
			{
				UE_LOG(LogTemp, Log, TEXT("Stage 3: Transforming data"));

				TArray<int32> InputData = FilterTask.GetResult();
				TArray<int32> TransformedData;

				for (int32 Value : InputData)
				{
					TransformedData.Add(Value * Value);
				}

				UE_LOG(LogTemp, Log, TEXT("Transformed %d values"), TransformedData.Num());
				return TransformedData;
			},
			UE::Tasks::Prerequisites(FilterTask)
		);

		// Stage 4: Aggregate (sum)
		auto AggregateTask = UE::Tasks::Launch(
			TEXT("AggregateData"),
			[TransformTask]() -> int32
			{
				UE_LOG(LogTemp, Log, TEXT("Stage 4: Aggregating data"));

				TArray<int32> InputData = TransformTask.GetResult();
				int32 Sum = 0;

				for (int32 Value : InputData)
				{
					Sum += Value;
				}

				UE_LOG(LogTemp, Log, TEXT("Final sum: %d"), Sum);
				return Sum;
			},
			UE::Tasks::Prerequisites(TransformTask)
		);

		// Wait for pipeline
		int32 FinalResult = AggregateTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("Pipeline complete! Result: %d"), FinalResult);
	}
};

/*
 * SOLUTION 4: Game Thread Safety Fix
 */

UCLASS()
class UExercise04_Component_Solution : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 ProcessedCount = 0;

	void ProcessDataSafe()
	{
		// Create weak pointer to self
		TWeakObjectPtr<UExercise04_Component_Solution> WeakThis(this);

		UE::Tasks::Launch(TEXT("SafeTask"), [WeakThis]()
		{
			// Background work - safe
			int32 ResultCount = 0;
			for (int32 i = 0; i < 100; i++)
			{
				FMath::Sin(static_cast<float>(i));
				ResultCount++;
			}

			// Return to game thread for UObject access
			AsyncTask(ENamedThreads::GameThread, [WeakThis, ResultCount]()
			{
				// Safe: On game thread
				if (WeakThis.IsValid())
				{
					WeakThis->ProcessedCount = ResultCount;
					UE_LOG(LogTemp, Log, TEXT("Safely updated ProcessedCount to %d"), ResultCount);
				}
			});
		});
	}
};

/*
 * SOLUTION 5: Optimal Batching Implementation
 */

class FExercise05_OptimalBatching_Solution
{
public:
	TArray<float> ProcessLargeDataset(const TArray<float>& Input)
	{
		if (Input.Num() == 0)
		{
			return TArray<float>();
		}

		// Calculate optimal batch size
		const int32 NumCores = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
		const int32 MinBatchSize = 1000;
		const int32 OptimalBatchSize = FMath::Max(MinBatchSize, Input.Num() / NumCores);

		const int32 NumBatches = (Input.Num() + OptimalBatchSize - 1) / OptimalBatchSize;

		UE_LOG(LogTemp, Log, TEXT("Processing %d items with %d cores, batch size %d (%d batches)"),
			Input.Num(), NumCores, OptimalBatchSize, NumBatches);

		// Results array per batch
		TArray<TArray<float>> BatchResults;
		BatchResults.SetNum(NumBatches);

		TArray<UE::Tasks::FTask> Tasks;

		// Create batched tasks
		for (int32 BatchIdx = 0; BatchIdx < NumBatches; BatchIdx++)
		{
			const int32 StartIdx = BatchIdx * OptimalBatchSize;
			const int32 EndIdx = FMath::Min(StartIdx + OptimalBatchSize, Input.Num());

			Tasks.Add(UE::Tasks::Launch(
				TEXT("ProcessBatch"),
				[this, &Input, &BatchResults, BatchIdx, StartIdx, EndIdx]()
				{
					for (int32 i = StartIdx; i < EndIdx; i++)
					{
						float Result = ComplexCalculation(Input[i]);
						BatchResults[BatchIdx].Add(Result);
					}

					UE_LOG(LogTemp, Log, TEXT("Batch %d complete (%d items)"),
						BatchIdx, BatchResults[BatchIdx].Num());
				}
			));
		}

		// Wait for all batches
		UE::Tasks::Wait(Tasks);

		// Merge results
		TArray<float> FinalResults;
		FinalResults.Reserve(Input.Num());

		for (const TArray<float>& Batch : BatchResults)
		{
			FinalResults.Append(Batch);
		}

		UE_LOG(LogTemp, Log, TEXT("Processing complete! %d results"), FinalResults.Num());
		return FinalResults;
	}

private:
	float ComplexCalculation(float Value)
	{
		return FMath::Sqrt(FMath::Sin(Value * Value));
	}
};

// Test function
inline void RunExercise01Solutions()
{
	UE_LOG(LogTemp, Log, TEXT("=== Running Exercise Solutions ===\n"));

	// Test parallel sum
	{
		FExercise01_ParallelSum_Solution Solution;
		TArray<int32> Numbers;
		for (int32 i = 1; i <= 100; i++)
		{
			Numbers.Add(i);
		}

		int32 Result = Solution.CalculateParallelSum(Numbers);
		UE_LOG(LogTemp, Log, TEXT("Parallel sum result: %d (expected 5050)\n"), Result);
	}

	// Test async file processing
	{
		FExercise02_AsyncFileProcessing_Solution Solution;
		TArray<FString> Files = {
			TEXT("File1.txt"),
			TEXT("File2.txt"),
			TEXT("File3.txt"),
			TEXT("File4.txt")
		};

		Solution.ProcessFilesAsync(Files);
		UE_LOG(LogTemp, Log, TEXT(""));
	}

	// Test pipeline
	{
		FExercise03_TaskPipeline_Solution Solution;
		Solution.RunPipeline();
		UE_LOG(LogTemp, Log, TEXT(""));
	}

	// Test optimal batching
	{
		FExercise05_OptimalBatching_Solution Solution;
		TArray<float> LargeDataset;
		for (int32 i = 0; i < 10000; i++)
		{
			LargeDataset.Add(static_cast<float>(i));
		}

		TArray<float> Results = Solution.ProcessLargeDataset(LargeDataset);
		UE_LOG(LogTemp, Log, TEXT(""));
	}
}
