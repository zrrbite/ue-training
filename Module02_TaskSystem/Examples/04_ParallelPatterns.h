// Example 4: Parallel Processing Patterns
// Demonstrates common parallel algorithms and patterns

#pragma once

#include "CoreMinimal.h"
#include "Tasks/Task.h"

class FParallelPatterns
{
public:
	// Example 1: Parallel For-Each
	void ParallelForEach()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Parallel For-Each ==="));

		// Data to process
		TArray<int32> Numbers;
		for (int32 i = 0; i < 1000; i++)
		{
			Numbers.Add(i);
		}

		// Determine number of tasks
		const int32 NumWorkers = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
		const int32 ItemsPerWorker = FMath::Max(1, Numbers.Num() / NumWorkers);

		UE_LOG(LogTemp, Log, TEXT("Processing %d items with %d workers (%d items each)"),
			Numbers.Num(), NumWorkers, ItemsPerWorker);

		// Results array (one per worker to avoid synchronization)
		TArray<TArray<int32>> PerWorkerResults;
		PerWorkerResults.SetNum(NumWorkers);

		TArray<UE::Tasks::FTask> Tasks;

		for (int32 WorkerIdx = 0; WorkerIdx < NumWorkers; WorkerIdx++)
		{
			const int32 StartIdx = WorkerIdx * ItemsPerWorker;
			const int32 EndIdx = (WorkerIdx == NumWorkers - 1)
				? Numbers.Num()
				: StartIdx + ItemsPerWorker;

			if (StartIdx < Numbers.Num())
			{
				Tasks.Add(UE::Tasks::Launch(
					TEXT("ParallelWorker"),
					[&Numbers, &PerWorkerResults, WorkerIdx, StartIdx, EndIdx]()
					{
						UE_LOG(LogTemp, Log, TEXT("Worker %d processing indices %d to %d"),
							WorkerIdx, StartIdx, EndIdx);

						for (int32 i = StartIdx; i < EndIdx; i++)
						{
							// Process item (square it)
							int32 Result = Numbers[i] * Numbers[i];
							PerWorkerResults[WorkerIdx].Add(Result);
						}

						UE_LOG(LogTemp, Log, TEXT("Worker %d complete"), WorkerIdx);
					}
				));
			}
		}

		// Wait for all workers
		UE::Tasks::Wait(Tasks);

		// Merge results
		TArray<int32> FinalResults;
		for (const TArray<int32>& WorkerResults : PerWorkerResults)
		{
			FinalResults.Append(WorkerResults);
		}

		UE_LOG(LogTemp, Log, TEXT("Parallel processing complete! %d results"), FinalResults.Num());
	}

	// Example 2: Map-Reduce Pattern
	void MapReduce()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Map-Reduce Pattern ==="));

		TArray<int32> Numbers;
		for (int32 i = 1; i <= 100; i++)
		{
			Numbers.Add(i);
		}

		const int32 NumWorkers = 4;
		const int32 ItemsPerWorker = Numbers.Num() / NumWorkers;

		// MAP PHASE: Process chunks in parallel
		TArray<UE::Tasks::FTask> MapTasks;

		for (int32 WorkerIdx = 0; WorkerIdx < NumWorkers; WorkerIdx++)
		{
			const int32 StartIdx = WorkerIdx * ItemsPerWorker;
			const int32 EndIdx = (WorkerIdx == NumWorkers - 1)
				? Numbers.Num()
				: StartIdx + ItemsPerWorker;

			MapTasks.Add(UE::Tasks::Launch(
				TEXT("MapTask"),
				[&Numbers, StartIdx, EndIdx]() -> int32
				{
					int32 LocalSum = 0;
					for (int32 i = StartIdx; i < EndIdx; i++)
					{
						LocalSum += Numbers[i] * Numbers[i];  // Square and sum
					}

					UE_LOG(LogTemp, Log, TEXT("Map task computed partial sum: %d"), LocalSum);
					return LocalSum;
				}
			));
		}

		// REDUCE PHASE: Combine results
		auto ReduceTask = UE::Tasks::Launch(
			TEXT("ReduceTask"),
			[MapTasks]() -> int32
			{
				int32 TotalSum = 0;

				for (const UE::Tasks::FTask& MapTask : MapTasks)
				{
					TotalSum += MapTask.GetResult();
				}

				UE_LOG(LogTemp, Log, TEXT("Reduce task computed total: %d"), TotalSum);
				return TotalSum;
			},
			UE::Tasks::Prerequisites(MapTasks)
		);

		int32 Result = ReduceTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("Map-Reduce complete! Sum of squares: %d"), Result);
	}

	// Example 3: Producer-Consumer Pattern
	void ProducerConsumer()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Producer-Consumer Pattern ==="));

		// Thread-safe queue
		TQueue<int32, EQueueMode::Mpsc> WorkQueue;  // Multi-producer, single-consumer
		std::atomic<bool> bProducingComplete{false};

		// Consumer task
		auto ConsumerTask = UE::Tasks::Launch(TEXT("Consumer"), [&WorkQueue, &bProducingComplete]()
		{
			UE_LOG(LogTemp, Log, TEXT("Consumer started"));

			TArray<int32> ProcessedItems;

			while (!bProducingComplete || !WorkQueue.IsEmpty())
			{
				int32 Item;
				if (WorkQueue.Dequeue(Item))
				{
					// Process item
					int32 Result = Item * Item;
					ProcessedItems.Add(Result);

					UE_LOG(LogTemp, Verbose, TEXT("Consumed item %d, result %d"), Item, Result);
				}
				else
				{
					FPlatformProcess::Sleep(0.001f);  // Brief sleep if queue empty
				}
			}

			UE_LOG(LogTemp, Log, TEXT("Consumer finished, processed %d items"), ProcessedItems.Num());
			return ProcessedItems;
		});

		// Producer tasks
		const int32 NumProducers = 3;
		TArray<UE::Tasks::FTask> ProducerTasks;

		for (int32 ProducerIdx = 0; ProducerIdx < NumProducers; ProducerIdx++)
		{
			ProducerTasks.Add(UE::Tasks::Launch(
				TEXT("Producer"),
				[&WorkQueue, ProducerIdx]()
				{
					UE_LOG(LogTemp, Log, TEXT("Producer %d started"), ProducerIdx);

					for (int32 i = 0; i < 10; i++)
					{
						int32 Item = ProducerIdx * 100 + i;
						WorkQueue.Enqueue(Item);

						UE_LOG(LogTemp, Verbose, TEXT("Producer %d enqueued %d"), ProducerIdx, Item);

						FPlatformProcess::Sleep(0.01f);  // Simulate work
					}

					UE_LOG(LogTemp, Log, TEXT("Producer %d finished"), ProducerIdx);
				}
			));
		}

		// Wait for all producers to finish
		UE::Tasks::Wait(ProducerTasks);
		bProducingComplete = true;

		// Wait for consumer
		TArray<int32> Results = ConsumerTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("Producer-Consumer complete! Total: %d items"), Results.Num());
	}

	// Example 4: Pipeline Pattern
	void Pipeline()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Pipeline Pattern ==="));

		// Three-stage pipeline: Generate → Transform → Aggregate

		TQueue<int32, EQueueMode::Spsc> Stage1To2Queue;
		TQueue<int32, EQueueMode::Spsc> Stage2To3Queue;

		std::atomic<bool> bStage1Complete{false};
		std::atomic<bool> bStage2Complete{false};

		// Stage 1: Generate numbers
		auto Stage1 = UE::Tasks::Launch(TEXT("Stage1_Generate"), [&Stage1To2Queue, &bStage1Complete]()
		{
			UE_LOG(LogTemp, Log, TEXT("Stage 1: Generating numbers"));

			for (int32 i = 1; i <= 20; i++)
			{
				Stage1To2Queue.Enqueue(i);
				FPlatformProcess::Sleep(0.01f);
			}

			bStage1Complete = true;
			UE_LOG(LogTemp, Log, TEXT("Stage 1: Complete"));
		});

		// Stage 2: Transform (square numbers)
		auto Stage2 = UE::Tasks::Launch(TEXT("Stage2_Transform"),
			[&Stage1To2Queue, &Stage2To3Queue, &bStage1Complete, &bStage2Complete]()
		{
			UE_LOG(LogTemp, Log, TEXT("Stage 2: Transforming numbers"));

			while (!bStage1Complete || !Stage1To2Queue.IsEmpty())
			{
				int32 Input;
				if (Stage1To2Queue.Dequeue(Input))
				{
					int32 Output = Input * Input;
					Stage2To3Queue.Enqueue(Output);
					UE_LOG(LogTemp, Verbose, TEXT("Stage 2: %d -> %d"), Input, Output);
				}
				else
				{
					FPlatformProcess::Sleep(0.001f);
				}
			}

			bStage2Complete = true;
			UE_LOG(LogTemp, Log, TEXT("Stage 2: Complete"));
		});

		// Stage 3: Aggregate (sum)
		auto Stage3 = UE::Tasks::Launch(TEXT("Stage3_Aggregate"),
			[&Stage2To3Queue, &bStage2Complete]() -> int32
		{
			UE_LOG(LogTemp, Log, TEXT("Stage 3: Aggregating results"));

			int32 Sum = 0;

			while (!bStage2Complete || !Stage2To3Queue.IsEmpty())
			{
				int32 Value;
				if (Stage2To3Queue.Dequeue(Value))
				{
					Sum += Value;
					UE_LOG(LogTemp, Verbose, TEXT("Stage 3: Sum = %d"), Sum);
				}
				else
				{
					FPlatformProcess::Sleep(0.001f);
				}
			}

			UE_LOG(LogTemp, Log, TEXT("Stage 3: Complete, Sum = %d"), Sum);
			return Sum;
		});

		// Wait for pipeline
		int32 FinalResult = Stage3.GetResult();
		UE_LOG(LogTemp, Log, TEXT("Pipeline complete! Final sum: %d"), FinalResult);
	}

	// Example 5: Batched parallel processing with optimal granularity
	void OptimalBatching()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Optimal Batching ==="));

		// Large dataset
		TArray<float> DataPoints;
		const int32 NumDataPoints = 1000000;

		for (int32 i = 0; i < NumDataPoints; i++)
		{
			DataPoints.Add(static_cast<float>(i));
		}

		// Calculate optimal batch size
		const int32 NumWorkers = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
		const int32 MinItemsPerBatch = 10000;  // Minimum to amortize task overhead
		const int32 OptimalBatchSize = FMath::Max(MinItemsPerBatch, NumDataPoints / NumWorkers);

		UE_LOG(LogTemp, Log, TEXT("Processing %d items, %d workers, batch size: %d"),
			NumDataPoints, NumWorkers, OptimalBatchSize);

		// Results (one array per batch to avoid synchronization)
		TArray<TArray<float>> BatchResults;
		const int32 NumBatches = (NumDataPoints + OptimalBatchSize - 1) / OptimalBatchSize;
		BatchResults.SetNum(NumBatches);

		TArray<UE::Tasks::FTask> Tasks;

		for (int32 BatchIdx = 0; BatchIdx < NumBatches; BatchIdx++)
		{
			const int32 StartIdx = BatchIdx * OptimalBatchSize;
			const int32 EndIdx = FMath::Min(StartIdx + OptimalBatchSize, NumDataPoints);

			Tasks.Add(UE::Tasks::Launch(
				TEXT("ProcessBatch"),
				[&DataPoints, &BatchResults, BatchIdx, StartIdx, EndIdx]()
				{
					// Complex computation per item
					for (int32 i = StartIdx; i < EndIdx; i++)
					{
						float Value = DataPoints[i];
						float Result = FMath::Sin(Value) * FMath::Cos(Value) + FMath::Sqrt(Value);
						BatchResults[BatchIdx].Add(Result);
					}
				}
			));
		}

		// Wait for all batches
		UE::Tasks::Wait(Tasks);

		// Merge results
		TArray<float> FinalResults;
		FinalResults.Reserve(NumDataPoints);

		for (const TArray<float>& Batch : BatchResults)
		{
			FinalResults.Append(Batch);
		}

		UE_LOG(LogTemp, Log, TEXT("Optimal batching complete! Processed %d items"), FinalResults.Num());
	}
};
