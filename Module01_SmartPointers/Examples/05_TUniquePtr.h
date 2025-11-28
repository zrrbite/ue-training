// Example 5: TUniquePtr - Exclusive Ownership
// Use for single ownership, RAII, and zero-overhead smart pointers

#pragma once

#include "CoreMinimal.h"

// Simple resource class
class FFileHandle
{
public:
	FString FileName;
	bool bIsOpen;

	explicit FFileHandle(const FString& Name)
		: FileName(Name)
		, bIsOpen(true)
	{
		UE_LOG(LogTemp, Log, TEXT("File opened: %s"), *FileName);
	}

	~FFileHandle()
	{
		Close();
	}

	void Close()
	{
		if (bIsOpen)
		{
			UE_LOG(LogTemp, Log, TEXT("File closed: %s"), *FileName);
			bIsOpen = false;
		}
	}

	void Write(const FString& Data)
	{
		if (bIsOpen)
		{
			UE_LOG(LogTemp, Log, TEXT("Writing to %s: %s"), *FileName, *Data);
		}
	}
};

// Example 1: RAII Pattern
class FFileWriter
{
private:
	TUniquePtr<FFileHandle> FileHandle;

public:
	explicit FFileWriter(const FString& FileName)
		: FileHandle(MakeUnique<FFileHandle>(FileName))
	{
		// File automatically opened in constructor
	}

	// Destructor automatically closes file via TUniquePtr
	~FFileWriter() = default;

	void WriteLine(const FString& Line)
	{
		if (FileHandle)
		{
			FileHandle->Write(Line);
		}
	}

	// Move semantics example
	FFileWriter(FFileWriter&& Other) = default;
	FFileWriter& operator=(FFileWriter&& Other) = default;

	// Deleted copy - unique ownership!
	FFileWriter(const FFileWriter&) = delete;
	FFileWriter& operator=(const FFileWriter&) = delete;
};

// Example 2: Pimpl (Pointer to Implementation) Pattern
class FComplexSystemImpl;  // Forward declaration

class FComplexSystem
{
private:
	// Hide implementation details
	TUniquePtr<FComplexSystemImpl> Impl;

public:
	FComplexSystem();
	~FComplexSystem();  // Must be defined in .cpp where FComplexSystemImpl is complete

	void DoWork();
	void ProcessData(const FString& Data);

	// Move operations
	FComplexSystem(FComplexSystem&& Other) noexcept;
	FComplexSystem& operator=(FComplexSystem&& Other) noexcept;

	// No copy operations
	FComplexSystem(const FComplexSystem&) = delete;
	FComplexSystem& operator=(const FComplexSystem&) = delete;
};

// Implementation (would be in .cpp file)
class FComplexSystemImpl
{
public:
	TArray<FString> InternalData;
	int32 ComplexState;

	FComplexSystemImpl()
		: ComplexState(0)
	{
		UE_LOG(LogTemp, Log, TEXT("ComplexSystemImpl created"));
	}

	~FComplexSystemImpl()
	{
		UE_LOG(LogTemp, Log, TEXT("ComplexSystemImpl destroyed"));
	}

	void Execute()
	{
		UE_LOG(LogTemp, Log, TEXT("Executing complex work..."));
		ComplexState++;
	}
};

// Constructor implementation
inline FComplexSystem::FComplexSystem()
	: Impl(MakeUnique<FComplexSystemImpl>())
{
}

inline FComplexSystem::~FComplexSystem() = default;

inline FComplexSystem::FComplexSystem(FComplexSystem&& Other) noexcept = default;
inline FComplexSystem& FComplexSystem::operator=(FComplexSystem&& Other) noexcept = default;

inline void FComplexSystem::DoWork()
{
	Impl->Execute();
}

inline void FComplexSystem::ProcessData(const FString& Data)
{
	Impl->InternalData.Add(Data);
}

// Example 3: Factory Pattern with Unique Ownership
class IProcessor
{
public:
	virtual ~IProcessor() = default;
	virtual void Process() = 0;
};

class FTextProcessor : public IProcessor
{
public:
	void Process() override
	{
		UE_LOG(LogTemp, Log, TEXT("Processing text..."));
	}
};

class FImageProcessor : public IProcessor
{
public:
	void Process() override
	{
		UE_LOG(LogTemp, Log, TEXT("Processing image..."));
	}
};

class FProcessorFactory
{
public:
	enum class EProcessorType
	{
		Text,
		Image
	};

	// Returns unique ownership to caller
	static TUniquePtr<IProcessor> CreateProcessor(EProcessorType Type)
	{
		switch (Type)
		{
		case EProcessorType::Text:
			return MakeUnique<FTextProcessor>();
		case EProcessorType::Image:
			return MakeUnique<FImageProcessor>();
		default:
			return nullptr;
		}
	}
};

// Example 4: Managing Arrays
class FArrayExample
{
public:
	void RunExample()
	{
		// Unique pointer to array
		TUniquePtr<int32[]> Numbers = MakeUnique<int32[]>(10);

		// Initialize
		for (int32 i = 0; i < 10; i++)
		{
			Numbers[i] = i * 10;
		}

		// Use
		for (int32 i = 0; i < 10; i++)
		{
			UE_LOG(LogTemp, Log, TEXT("Numbers[%d] = %d"), i, Numbers[i]);
		}

		// Automatically deleted when Numbers goes out of scope
	}
};

// Example 5: Custom Deleters
class FResourceWithCleanup
{
public:
	FString ResourceName;

	explicit FResourceWithCleanup(const FString& Name)
		: ResourceName(Name)
	{
		UE_LOG(LogTemp, Log, TEXT("Resource acquired: %s"), *ResourceName);
	}

	~FResourceWithCleanup()
	{
		UE_LOG(LogTemp, Log, TEXT("Resource released: %s"), *ResourceName);
	}
};

// Custom deleter
struct FCustomDeleter
{
	void operator()(FResourceWithCleanup* Resource) const
	{
		UE_LOG(LogTemp, Log, TEXT("Custom cleanup for: %s"), *Resource->ResourceName);
		delete Resource;
	}
};

// Example usage
class FUniquePtrExamples
{
public:
	void RAIIExample()
	{
		UE_LOG(LogTemp, Log, TEXT("=== RAII Example ==="));

		{
			FFileWriter Writer(TEXT("output.txt"));
			Writer.WriteLine(TEXT("Hello, World!"));
			Writer.WriteLine(TEXT("Another line"));
		}  // File automatically closed here - no manual cleanup!

		UE_LOG(LogTemp, Log, TEXT("File has been closed automatically"));
	}

	void MoveSemantics()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Move Semantics Example ==="));

		TUniquePtr<FFileHandle> File1 = MakeUnique<FFileHandle>(TEXT("file1.txt"));

		// Move ownership - File1 becomes null
		TUniquePtr<FFileHandle> File2 = MoveTemp(File1);

		check(File1 == nullptr);  // File1 no longer owns anything
		check(File2 != nullptr);  // File2 now owns the file

		// Cannot copy - this won't compile:
		// TUniquePtr<FFileHandle> File3 = File2;  // ERROR!
	}

	void FactoryExample()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Factory Example ==="));

		// Get unique ownership from factory
		TUniquePtr<IProcessor> Processor =
			FProcessorFactory::CreateProcessor(FProcessorFactory::EProcessorType::Text);

		if (Processor)
		{
			Processor->Process();
		}

		// Transfer ownership to another variable
		TUniquePtr<IProcessor> AnotherOwner = MoveTemp(Processor);

		// Processor is now null, AnotherOwner owns the object
		check(Processor == nullptr);
		AnotherOwner->Process();
	}

	void PimplExample()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Pimpl Example ==="));

		FComplexSystem System;
		System.DoWork();
		System.ProcessData(TEXT("Test data"));

		// Move to another owner
		FComplexSystem MovedSystem = MoveTemp(System);
		MovedSystem.DoWork();

		// Implementation details hidden, automatic cleanup
	}

	void ResetAndRelease()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Reset and Release Example ==="));

		TUniquePtr<FFileHandle> File = MakeUnique<FFileHandle>(TEXT("test.txt"));

		// Release ownership without deleting
		FFileHandle* RawPtr = File.Release();
		check(File == nullptr);  // File no longer owns anything

		// Now we're responsible for manual deletion
		delete RawPtr;

		// Reset with new value
		File = MakeUnique<FFileHandle>(TEXT("another.txt"));

		// Reset to null (deletes current object)
		File.Reset();
		check(File == nullptr);
	}

	void CustomDeleterExample()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Custom Deleter Example ==="));

		TUniquePtr<FResourceWithCleanup, FCustomDeleter> Resource(
			new FResourceWithCleanup(TEXT("CustomResource")));

		// When Resource goes out of scope, FCustomDeleter is called
	}

	void CompareWithRawPointer()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Comparison with Raw Pointer ==="));

		// Old way - manual management
		{
			FFileHandle* RawFile = new FFileHandle(TEXT("raw.txt"));
			RawFile->Write(TEXT("Data"));

			// Easy to forget this!
			delete RawFile;
		}

		// Modern way - automatic management
		{
			TUniquePtr<FFileHandle> UniqueFile = MakeUnique<FFileHandle>(TEXT("unique.txt"));
			UniqueFile->Write(TEXT("Data"));

			// Automatically deleted - can't forget!
		}
	}

	void PassToFunction()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Passing to Functions ==="));

		TUniquePtr<FFileHandle> File = MakeUnique<FFileHandle>(TEXT("func.txt"));

		// Pass by reference - no ownership transfer
		UseFile(File);
		check(File != nullptr);  // Still owns it

		// Transfer ownership
		ConsumeFile(MoveTemp(File));
		check(File == nullptr);  // No longer owns it
	}

	TUniquePtr<FFileHandle> ReturnFromFunction()
	{
		// Return unique_ptr - automatic move
		return MakeUnique<FFileHandle>(TEXT("returned.txt"));
	}

private:
	void UseFile(const TUniquePtr<FFileHandle>& File)
	{
		// Use file, but don't take ownership
		if (File)
		{
			File->Write(TEXT("Used but not owned"));
		}
	}

	void ConsumeFile(TUniquePtr<FFileHandle> File)
	{
		// Takes ownership
		if (File)
		{
			File->Write(TEXT("Consumed"));
		}
		// File is deleted when function exits
	}
};

// Example 6: Container of Unique Pointers
class FContainerExample
{
public:
	void RunExample()
	{
		TArray<TUniquePtr<IProcessor>> Processors;

		// Add processors
		Processors.Add(FProcessorFactory::CreateProcessor(FProcessorFactory::EProcessorType::Text));
		Processors.Add(FProcessorFactory::CreateProcessor(FProcessorFactory::EProcessorType::Image));

		// Process all
		for (const TUniquePtr<IProcessor>& Processor : Processors)
		{
			Processor->Process();
		}

		// All automatically cleaned up when array is destroyed
	}
};
