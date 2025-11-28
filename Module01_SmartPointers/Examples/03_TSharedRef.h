// Example 3: TSharedRef - Shared Ownership That Cannot Be Null
// Use when null is not a valid state

#pragma once

#include "CoreMinimal.h"

// Configuration data that should always exist
struct FGameConfig
{
	float MasterVolume = 1.0f;
	float MouseSensitivity = 1.0f;
	bool bInvertY = false;
	FString PlayerName = TEXT("Player");

	void ApplySettings()
	{
		UE_LOG(LogTemp, Log, TEXT("Applying settings for %s"), *PlayerName);
		UE_LOG(LogTemp, Log, TEXT("Volume: %.2f, Sensitivity: %.2f, InvertY: %s"),
			MasterVolume, MouseSensitivity, bInvertY ? TEXT("Yes") : TEXT("No"));
	}
};

// System that requires valid config
class FAudioSystem
{
private:
	TSharedRef<FGameConfig> Config;

public:
	// Constructor REQUIRES a valid config - cannot be null
	explicit FAudioSystem(TSharedRef<FGameConfig> InConfig)
		: Config(InConfig)
	{
		// No need to check IsValid() - guaranteed to be valid!
		UE_LOG(LogTemp, Log, TEXT("AudioSystem initialized with volume: %.2f"), Config->MasterVolume);
	}

	void UpdateVolume()
	{
		// Direct use - no null checking needed
		float CurrentVolume = Config->MasterVolume;
		UE_LOG(LogTemp, Log, TEXT("Current volume: %.2f"), CurrentVolume);
	}

	void SetVolume(float NewVolume)
	{
		Config->MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	}
};

class FInputSystem
{
private:
	TSharedRef<FGameConfig> Config;

public:
	explicit FInputSystem(TSharedRef<FGameConfig> InConfig)
		: Config(InConfig)
	{
		// Guaranteed valid - no defensive programming needed
	}

	void ProcessInput(float MouseDelta)
	{
		// Direct access without null checks
		float AdjustedDelta = MouseDelta * Config->MouseSensitivity;

		if (Config->bInvertY)
		{
			AdjustedDelta *= -1.0f;
		}

		UE_LOG(LogTemp, Log, TEXT("Adjusted input: %.2f"), AdjustedDelta);
	}
};

// Example usage
class FSharedRefExample
{
public:
	void RunExample()
	{
		// Create a TSharedRef - must be initialized with a value
		TSharedRef<FGameConfig> Config = MakeShared<FGameConfig>();

		// Cannot do this - won't compile:
		// TSharedRef<FGameConfig> Invalid;  // ERROR!

		// Share with multiple systems
		FAudioSystem AudioSys(Config);
		FInputSystem InputSys(Config);

		// Modify config in one place
		Config->MasterVolume = 0.75f;
		Config->MouseSensitivity = 1.5f;

		// All systems see the changes
		AudioSys.UpdateVolume();
		InputSys.ProcessInput(10.0f);
	}

	void ConversionExample()
	{
		TSharedRef<FGameConfig> ConfigRef = MakeShared<FGameConfig>();

		// Can convert TSharedRef to TSharedPtr (always safe)
		TSharedPtr<FGameConfig> ConfigPtr = ConfigRef;

		// Cannot directly convert TSharedPtr to TSharedRef
		TSharedPtr<FGameConfig> MaybeNull = MakeShared<FGameConfig>();

		// Must use ToSharedRef() which asserts if null
		if (MaybeNull.IsValid())
		{
			TSharedRef<FGameConfig> Converted = MaybeNull.ToSharedRef();
			// Now it's a reference
		}
	}

	// Good API design: Function that requires valid config
	void ApplyConfiguration(TSharedRef<FGameConfig> Config)
	{
		// No null checking needed - contract is clear
		Config->ApplySettings();

		// Callers cannot pass null - compile error
		// This makes the API safer and clearer
	}

	// Compare with TSharedPtr version
	void ApplyConfigurationPtr(TSharedPtr<FGameConfig> Config)
	{
		// Must do defensive null checking
		if (Config.IsValid())
		{
			Config->ApplySettings();
		}
		else
		{
			// What to do here? Error? Default config?
			UE_LOG(LogTemp, Error, TEXT("Null config passed!"));
		}
	}

	void DemonstrateAPIDesign()
	{
		TSharedRef<FGameConfig> Config = MakeShared<FGameConfig>();

		// Clear and safe - cannot pass null
		ApplyConfiguration(Config);

		// Less clear - might be null, requires checking
		TSharedPtr<FGameConfig> PtrConfig = Config;
		ApplyConfigurationPtr(PtrConfig);

		// This won't compile - good! Catches bugs at compile time
		// ApplyConfiguration(nullptr);  // ERROR!

		// This compiles but might fail at runtime - bad!
		ApplyConfigurationPtr(nullptr);  // Compiles, runtime error
	}
};

// Factory pattern with TSharedRef
class FConfigFactory
{
public:
	// Returns TSharedRef - guarantees a valid config is returned
	static TSharedRef<FGameConfig> CreateDefaultConfig()
	{
		TSharedRef<FGameConfig> Config = MakeShared<FGameConfig>();
		Config->PlayerName = TEXT("DefaultPlayer");
		Config->MasterVolume = 0.8f;
		return Config;
	}

	// Compare with TSharedPtr version
	static TSharedPtr<FGameConfig> LoadConfig(const FString& FilePath)
	{
		// Might fail to load - can return null
		if (FilePath.IsEmpty())
		{
			return nullptr;  // Indicates failure
		}

		return MakeShared<FGameConfig>();
	}

	// If a factory should always succeed, use TSharedRef
	static TSharedRef<FGameConfig> LoadOrCreateConfig(const FString& FilePath)
	{
		// Try to load
		if (!FilePath.IsEmpty())
		{
			// Load logic here...
			return MakeShared<FGameConfig>();
		}

		// Fall back to default - always returns valid config
		return CreateDefaultConfig();
	}
};

// Real-world example: Subsystem with guaranteed dependencies
class FGameSubsystem
{
private:
	TSharedRef<FGameConfig> Config;
	TSharedRef<FAudioSystem> AudioSystem;

public:
	// Constructor requires all dependencies - no null states possible
	FGameSubsystem(TSharedRef<FGameConfig> InConfig)
		: Config(InConfig)
		, AudioSystem(MakeShared<FAudioSystem>(InConfig))
	{
		// All members are guaranteed valid
		// No need for lazy initialization or null checks
	}

	void Initialize()
	{
		// Direct use of all systems - no null checking
		Config->ApplySettings();
		AudioSystem->UpdateVolume();
	}

	// Getter that maintains the guarantee
	TSharedRef<FGameConfig> GetConfig() const
	{
		return Config;
	}
};
