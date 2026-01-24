// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OverHeadStatusGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"
#include "GAS/CAttributeSet.h"

void UOverHeadStatusGauge::ConfigureWithASC(UAbilitySystemComponent* InASC)
{
	if (InASC)
	{
		HealthBar->SetAndBoundToGameplayAttribute(InASC, UCAttributeSet::GetHealthAttribute(), UCAttributeSet::GetMaxHealthAttribute());
		ManaBar->SetAndBoundToGameplayAttribute(InASC, UCAttributeSet::GetManaAttribute(), UCAttributeSet::GetMaxManaAttribute());
	}
}
