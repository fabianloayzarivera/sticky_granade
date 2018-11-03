// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Stickable.generated.h"

UINTERFACE(MinimalAPI)
class UStickable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GRANADETEST_API IStickable
{
	GENERATED_IINTERFACE_BODY()

public:
	UFUNCTION()
		virtual void OnStick() = 0;
	
};
