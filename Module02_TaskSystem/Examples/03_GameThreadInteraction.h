// Example 3: Game Thread Interaction
// Demonstrates proper UObject access patterns with tasks

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tasks/Task.h"
#include "03_GameThreadInteraction.generated.h"

// Example data structure (not a UObject - safe for background threads)
struct FComputationResult
{
	TArray<FVector> Positions;
	TArray<FRotator> Rotations;
	float AverageDistance;
};

UCLASS()
class ATaskExampleActor : public AActor
{
	GENERATED_BODY()

public:
	// Example 1: Background computation with game thread callback
	void PerformBackgroundComputation()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Background Computation Example ==="));

		// Get data from UObject on game thread
		FVector StartLocation = GetActorLocation();
		int32 NumPoints = 100;

		// Launch background task
		UE::Tasks::Launch(TEXT("ComputePositions"), [StartLocation, NumPoints]()
		{
			// BACKGROUND THREAD - No UObject access!
			UE_LOG(LogTemp, Log, TEXT("Computing %d positions on background thread"), NumPoints);

			TArray<FVector> ComputedPositions;
			for (int32 i = 0; i < NumPoints; i++)
			{
				FVector Offset = FVector(
					FMath::RandRange(-100.0f, 100.0f),
					FMath::RandRange(-100.0f, 100.0f),
					FMath::RandRange(-100.0f, 100.0f)
				);

				ComputedPositions.Add(StartLocation + Offset);
			}

			// Return to game thread to use results
			AsyncTask(ENamedThreads::GameThread, [ComputedPositions]()
			{
				// GAME THREAD - Safe for UObjects
				UE_LOG(LogTemp, Log, TEXT("Back on game thread with %d positions"), ComputedPositions.Num());

				// Could spawn actors, modify components, etc.
				// SpawnActorsAtPositions(ComputedPositions);
			});
		});
	}

	// Example 2: Safe UObject access with weak pointers
	void SafeAsyncOperation()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Safe Async with Weak Pointers ==="));

		// Create weak pointer to self
		TWeakObjectPtr<ATaskExampleActor> WeakThis(this);

		UE::Tasks::Launch(TEXT("SafeOperation"), [WeakThis]()
		{
			// Background work
			UE_LOG(LogTemp, Log, TEXT("Performing background calculation..."));
			FPlatformProcess::Sleep(0.5f);

			TArray<int32> Results = {1, 2, 3, 4, 5};

			// Return to game thread
			AsyncTask(ENamedThreads::GameThread, [WeakThis, Results]()
			{
				// Check if object still exists
				if (WeakThis.IsValid())
				{
					UE_LOG(LogTemp, Log, TEXT("Actor still exists, applying results"));
					// Safe to access member functions
					// WeakThis->ApplyResults(Results);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Actor was destroyed, discarding results"));
				}
			});
		});
	}

	// Example 3: Multiple stages with game thread synchronization
	void MultiStageProcessing()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Multi-Stage Processing ==="));

		TWeakObjectPtr<ATaskExampleActor> WeakThis(this);
		FVector CurrentLocation = GetActorLocation();

		// Stage 1: Background processing
		UE::Tasks::Launch(TEXT("Stage1"), [WeakThis, CurrentLocation]()
		{
			UE_LOG(LogTemp, Log, TEXT("Stage 1: Background processing"));

			// Heavy computation
			TArray<FVector> ProcessedData;
			for (int32 i = 0; i < 50; i++)
			{
				ProcessedData.Add(CurrentLocation + FVector(i * 10.0f, 0, 0));
			}

			FPlatformProcess::Sleep(0.1f);

			// Stage 2: Return to game thread for UObject access
			AsyncTask(ENamedThreads::GameThread, [WeakThis, ProcessedData]()
			{
				if (!WeakThis.IsValid())
				{
					return;
				}

				UE_LOG(LogTemp, Log, TEXT("Stage 2: On game thread, reading UObject data"));

				// Could access UObject properties here
				// float SomeProperty = WeakThis->GetSomeValue();

				// Stage 3: Back to background thread for more processing
				UE::Tasks::Launch(TEXT("Stage3"), [ProcessedData]()
				{
					UE_LOG(LogTemp, Log, TEXT("Stage 3: More background processing"));

					TArray<float> Distances;
					for (const FVector& Pos : ProcessedData)
					{
						Distances.Add(Pos.Size());
					}

					FPlatformProcess::Sleep(0.1f);

					// Stage 4: Final game thread callback
					AsyncTask(ENamedThreads::GameThread, [Distances]()
					{
						UE_LOG(LogTemp, Log, TEXT("Stage 4: Final results on game thread"));
						UE_LOG(LogTemp, Log, TEXT("Processed %d distance values"), Distances.Num());
					});
				});
			});
		});
	}

	// Example 4: Parallel processing with game thread aggregation
	void ParallelProcessing()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Parallel Processing ==="));

		TWeakObjectPtr<ATaskExampleActor> WeakThis(this);

		// Launch multiple parallel tasks
		const int32 NumTasks = 4;
		TArray<UE::Tasks::FTask> Tasks;

		for (int32 TaskIdx = 0; TaskIdx < NumTasks; TaskIdx++)
		{
			Tasks.Add(UE::Tasks::Launch(TEXT("ParallelWorker"), [TaskIdx]() -> TArray<int32>
			{
				UE_LOG(LogTemp, Log, TEXT("Worker %d starting"), TaskIdx);

				TArray<int32> LocalResults;
				for (int32 i = 0; i < 100; i++)
				{
					LocalResults.Add(TaskIdx * 1000 + i);
				}

				FPlatformProcess::Sleep(0.1f);
				UE_LOG(LogTemp, Log, TEXT("Worker %d complete"), TaskIdx);

				return LocalResults;
			}));
		}

		// Aggregation task that waits for all workers
		UE::Tasks::Launch(
			TEXT("Aggregator"),
			[WeakThis, Tasks]()
			{
				// Wait for all workers
				UE::Tasks::Wait(Tasks);

				// Collect results
				TArray<int32> AllResults;
				for (const UE::Tasks::FTask& Task : Tasks)
				{
					AllResults.Append(Task.GetResult());
				}

				UE_LOG(LogTemp, Log, TEXT("All workers complete, collected %d results"), AllResults.Num());

				// Return to game thread
				AsyncTask(ENamedThreads::GameThread, [WeakThis, AllResults]()
				{
					if (WeakThis.IsValid())
					{
						UE_LOG(LogTemp, Log, TEXT("Applying %d results to actor"), AllResults.Num());
						// Could modify actor here
					}
				});
			},
			UE::Tasks::Prerequisites(Tasks)
		);
	}

	// Example 5: Legacy AsyncTask pattern
	void LegacyAsyncPattern()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Legacy AsyncTask Pattern ==="));

		TWeakObjectPtr<ATaskExampleActor> WeakThis(this);
		FVector StartPos = GetActorLocation();

		// Launch on background thread
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis, StartPos]()
		{
			UE_LOG(LogTemp, Log, TEXT("Legacy async task on background thread"));

			// Process data
			TArray<FVector> Results;
			for (int32 i = 0; i < 10; i++)
			{
				Results.Add(StartPos + FVector(i * 50.0f, 0, 0));
			}

			FPlatformProcess::Sleep(0.1f);

			// Return to game thread
			AsyncTask(ENamedThreads::GameThread, [WeakThis, Results]()
			{
				if (WeakThis.IsValid())
				{
					UE_LOG(LogTemp, Log, TEXT("Back on game thread"));
					// Safe to access UObject members
				}
			});
		});
	}

	// Example 6: Dangerous pattern - DON'T DO THIS!
	void DangerousPattern_Example()
	{
		UE_LOG(LogTemp, Warning, TEXT("=== DANGEROUS PATTERN - FOR DEMONSTRATION ONLY ==="));

		/* WRONG! This will crash!
		UE::Tasks::Launch(TEXT("UnsafeTask"), [this]()
		{
			// CRASH! Accessing UObject from background thread
			SetActorLocation(FVector::ZeroVector);
		});
		*/

		// The correct way:
		UE::Tasks::Launch(TEXT("SafeTask"), [this]()
		{
			// Background work
			FVector NewLocation = FVector(100, 200, 300);

			// Return to game thread for UObject access
			AsyncTask(ENamedThreads::GameThread, [this, NewLocation]()
			{
				// Now safe
				if (IsValid(this))
				{
					SetActorLocation(NewLocation);
				}
			});
		});
	}
};

// Component example with async loading
UCLASS()
class UAsyncLoaderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Load data asynchronously
	void LoadDataAsync(const FString& FilePath)
	{
		UE_LOG(LogTemp, Log, TEXT("=== Async Data Loading ==="));

		TWeakObjectPtr<UAsyncLoaderComponent> WeakThis(this);

		// Background: Load from disk
		UE::Tasks::Launch(TEXT("LoadFile"), [WeakThis, FilePath]()
		{
			UE_LOG(LogTemp, Log, TEXT("Loading file: %s"), *FilePath);

			// Simulate file loading
			TArray<uint8> FileData;
			// FFileHelper::LoadFileToArray(FileData, *FilePath);

			FPlatformProcess::Sleep(0.2f);

			// Background: Parse data
			UE_LOG(LogTemp, Log, TEXT("Parsing data..."));
			TArray<FString> ParsedLines;
			// Parse FileData into ParsedLines

			FPlatformProcess::Sleep(0.1f);

			// Game thread: Apply to component
			AsyncTask(ENamedThreads::GameThread, [WeakThis, ParsedLines]()
			{
				if (WeakThis.IsValid())
				{
					UE_LOG(LogTemp, Log, TEXT("Data loaded and parsed, applying to component"));
					// WeakThis->ApplyData(ParsedLines);
					// WeakThis->OnLoadComplete.Broadcast();
				}
			});
		});
	}
};
