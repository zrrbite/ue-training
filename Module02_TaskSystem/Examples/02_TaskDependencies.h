// Example 2: Task Dependencies and Prerequisites
// Demonstrates building task graphs with dependencies

#pragma once

#include "CoreMinimal.h"
#include "Tasks/Task.h"

class FTaskDependencyExamples
{
public:
	// Example 1: Simple dependency chain
	void SimpleDependency()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Simple Dependency Chain ==="));

		// Task 1: Load data
		auto LoadTask = UE::Tasks::Launch(TEXT("LoadData"), []() -> TArray<int32>
		{
			UE_LOG(LogTemp, Log, TEXT("Loading data..."));
			FPlatformProcess::Sleep(0.1f);

			TArray<int32> Data = {10, 20, 30, 40, 50};
			UE_LOG(LogTemp, Log, TEXT("Data loaded: %d items"), Data.Num());

			return Data;
		});

		// Task 2: Process data (depends on Task 1)
		auto ProcessTask = UE::Tasks::Launch(
			TEXT("ProcessData"),
			[LoadTask]() -> TArray<int32>
			{
				UE_LOG(LogTemp, Log, TEXT("Processing data..."));

				TArray<int32> LoadedData = LoadTask.GetResult();
				TArray<int32> ProcessedData;

				// Double each value
				for (int32 Value : LoadedData)
				{
					ProcessedData.Add(Value * 2);
				}

				UE_LOG(LogTemp, Log, TEXT("Data processed: %d items"), ProcessedData.Num());
				return ProcessedData;
			},
			UE::Tasks::Prerequisites(LoadTask)  // Depends on LoadTask
		);

		// Task 3: Save data (depends on Task 2)
		auto SaveTask = UE::Tasks::Launch(
			TEXT("SaveData"),
			[ProcessTask]()
			{
				UE_LOG(LogTemp, Log, TEXT("Saving data..."));

				TArray<int32> ProcessedData = ProcessTask.GetResult();

				UE_LOG(LogTemp, Log, TEXT("Saved %d values"), ProcessedData.Num());
				for (int32 Value : ProcessedData)
				{
					UE_LOG(LogTemp, Log, TEXT("  Value: %d"), Value);
				}
			},
			UE::Tasks::Prerequisites(ProcessTask)  // Depends on ProcessTask
		);

		// Wait for entire pipeline
		SaveTask.Wait();
		UE_LOG(LogTemp, Log, TEXT("Pipeline complete!"));
	}

	// Example 2: Fan-out pattern (one task → multiple tasks)
	void FanOutPattern()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Fan-Out Pattern ==="));

		// Source task
		auto SourceTask = UE::Tasks::Launch(TEXT("Source"), []() -> TArray<int32>
		{
			UE_LOG(LogTemp, Log, TEXT("Generating source data..."));
			return TArray<int32>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		});

		// Multiple workers process different parts
		auto Worker1 = UE::Tasks::Launch(
			TEXT("Worker1"),
			[SourceTask]() -> int32
			{
				TArray<int32> Data = SourceTask.GetResult();
				int32 Sum = 0;
				for (int32 Value : Data)
				{
					Sum += Value;
				}
				UE_LOG(LogTemp, Log, TEXT("Worker1 calculated sum: %d"), Sum);
				return Sum;
			},
			UE::Tasks::Prerequisites(SourceTask)
		);

		auto Worker2 = UE::Tasks::Launch(
			TEXT("Worker2"),
			[SourceTask]() -> int32
			{
				TArray<int32> Data = SourceTask.GetResult();
				int32 Product = 1;
				for (int32 Value : Data)
				{
					Product *= Value;
				}
				UE_LOG(LogTemp, Log, TEXT("Worker2 calculated product: %d"), Product);
				return Product;
			},
			UE::Tasks::Prerequisites(SourceTask)
		);

		auto Worker3 = UE::Tasks::Launch(
			TEXT("Worker3"),
			[SourceTask]() -> int32
			{
				TArray<int32> Data = SourceTask.GetResult();
				int32 Max = Data.Num() > 0 ? Data[0] : 0;
				for (int32 Value : Data)
				{
					Max = FMath::Max(Max, Value);
				}
				UE_LOG(LogTemp, Log, TEXT("Worker3 found max: %d"), Max);
				return Max;
			},
			UE::Tasks::Prerequisites(SourceTask)
		);

		// Wait for all workers
		UE::Tasks::Wait(TArray{Worker1, Worker2, Worker3});
		UE_LOG(LogTemp, Log, TEXT("All workers complete!"));
	}

	// Example 3: Fan-in pattern (multiple tasks → one task)
	void FanInPattern()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Fan-In Pattern ==="));

		// Multiple source tasks
		auto Source1 = UE::Tasks::Launch(TEXT("Source1"), []() -> int32
		{
			UE_LOG(LogTemp, Log, TEXT("Source1 generating..."));
			FPlatformProcess::Sleep(0.1f);
			return 10;
		});

		auto Source2 = UE::Tasks::Launch(TEXT("Source2"), []() -> int32
		{
			UE_LOG(LogTemp, Log, TEXT("Source2 generating..."));
			FPlatformProcess::Sleep(0.15f);
			return 20;
		});

		auto Source3 = UE::Tasks::Launch(TEXT("Source3"), []() -> int32
		{
			UE_LOG(LogTemp, Log, TEXT("Source3 generating..."));
			FPlatformProcess::Sleep(0.05f);
			return 30;
		});

		// Combine task waits for all sources
		auto CombineTask = UE::Tasks::Launch(
			TEXT("Combine"),
			[Source1, Source2, Source3]() -> int32
			{
				int32 Result1 = Source1.GetResult();
				int32 Result2 = Source2.GetResult();
				int32 Result3 = Source3.GetResult();

				int32 Total = Result1 + Result2 + Result3;

				UE_LOG(LogTemp, Log, TEXT("Combined results: %d + %d + %d = %d"),
					Result1, Result2, Result3, Total);

				return Total;
			},
			UE::Tasks::Prerequisites(Source1, Source2, Source3)
		);

		int32 FinalResult = CombineTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("Final result: %d"), FinalResult);
	}

	// Example 4: Complex dependency graph (diamond pattern)
	void DiamondDependency()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Diamond Dependency Pattern ==="));

		// Top: Initial task
		auto TopTask = UE::Tasks::Launch(TEXT("Top"), []() -> int32
		{
			UE_LOG(LogTemp, Log, TEXT("Top task"));
			return 100;
		});

		// Left: Depends on Top
		auto LeftTask = UE::Tasks::Launch(
			TEXT("Left"),
			[TopTask]() -> int32
			{
				int32 Value = TopTask.GetResult();
				UE_LOG(LogTemp, Log, TEXT("Left task, input: %d"), Value);
				return Value + 10;
			},
			UE::Tasks::Prerequisites(TopTask)
		);

		// Right: Depends on Top
		auto RightTask = UE::Tasks::Launch(
			TEXT("Right"),
			[TopTask]() -> int32
			{
				int32 Value = TopTask.GetResult();
				UE_LOG(LogTemp, Log, TEXT("Right task, input: %d"), Value);
				return Value + 20;
			},
			UE::Tasks::Prerequisites(TopTask)
		);

		// Bottom: Depends on both Left and Right
		auto BottomTask = UE::Tasks::Launch(
			TEXT("Bottom"),
			[LeftTask, RightTask]() -> int32
			{
				int32 LeftValue = LeftTask.GetResult();
				int32 RightValue = RightTask.GetResult();

				int32 Result = LeftValue + RightValue;

				UE_LOG(LogTemp, Log, TEXT("Bottom task: %d + %d = %d"),
					LeftValue, RightValue, Result);

				return Result;
			},
			UE::Tasks::Prerequisites(LeftTask, RightTask)
		);

		int32 Final = BottomTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("Diamond complete! Result: %d"), Final);
	}

	// Example 5: Pipeline with filtering
	void PipelineWithFiltering()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Pipeline with Filtering ==="));

		// Stage 1: Generate data
		auto GenerateTask = UE::Tasks::Launch(TEXT("Generate"), []() -> TArray<int32>
		{
			UE_LOG(LogTemp, Log, TEXT("Generating numbers..."));
			TArray<int32> Numbers;
			for (int32 i = 1; i <= 20; i++)
			{
				Numbers.Add(i);
			}
			return Numbers;
		});

		// Stage 2: Filter even numbers
		auto FilterTask = UE::Tasks::Launch(
			TEXT("Filter"),
			[GenerateTask]() -> TArray<int32>
			{
				TArray<int32> Input = GenerateTask.GetResult();
				TArray<int32> Output;

				for (int32 Num : Input)
				{
					if (Num % 2 == 0)
					{
						Output.Add(Num);
					}
				}

				UE_LOG(LogTemp, Log, TEXT("Filtered to %d even numbers"), Output.Num());
				return Output;
			},
			UE::Tasks::Prerequisites(GenerateTask)
		);

		// Stage 3: Square the numbers
		auto SquareTask = UE::Tasks::Launch(
			TEXT("Square"),
			[FilterTask]() -> TArray<int32>
			{
				TArray<int32> Input = FilterTask.GetResult();
				TArray<int32> Output;

				for (int32 Num : Input)
				{
					Output.Add(Num * Num);
				}

				UE_LOG(LogTemp, Log, TEXT("Squared %d numbers"), Output.Num());
				return Output;
			},
			UE::Tasks::Prerequisites(FilterTask)
		);

		// Stage 4: Sum the results
		auto SumTask = UE::Tasks::Launch(
			TEXT("Sum"),
			[SquareTask]() -> int32
			{
				TArray<int32> Input = SquareTask.GetResult();
				int32 Sum = 0;

				for (int32 Num : Input)
				{
					Sum += Num;
				}

				UE_LOG(LogTemp, Log, TEXT("Sum of squared evens: %d"), Sum);
				return Sum;
			},
			UE::Tasks::Prerequisites(SquareTask)
		);

		int32 Result = SumTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("Pipeline result: %d"), Result);
	}
};
