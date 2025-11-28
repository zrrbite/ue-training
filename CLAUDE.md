# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository is for Unreal Engine training and practice projects.

## Unreal Engine C++ Guidelines

When working with Unreal Engine C++ code:
- Follow Unreal Engine coding standards (PascalCase for classes/functions, prefix U/A/F/E/I as appropriate)
- Use Unreal's reflection system (UCLASS, UPROPERTY, UFUNCTION) appropriately
- Prefer Unreal's container types (TArray, TMap, TSet) over STL equivalents
- Use smart pointers appropriately (TSharedPtr, TWeakPtr, TUniquePtr for non-UObject types)
- Be mindful of garbage collection - UObjects should not be manually deleted
- Include proper GENERATED_BODY() macros in classes using reflection
- Use forward declarations to minimize header dependencies

## Development Commands

Commands will be added as projects are created. Typical Unreal Engine workflows include:
- Building through Visual Studio or Rider
- Using Unreal Build Tool (UBT) from command line
- Generating project files after adding modules/plugins
- Running automation tests

## Architecture

Project-specific architecture will be documented as training projects are developed.
