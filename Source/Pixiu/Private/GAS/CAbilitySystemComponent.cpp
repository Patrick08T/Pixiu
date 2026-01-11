// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemComponent.h"

void UCAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for(const TSubclassOf<UGameplayEffect>& EffectClass : InitialEffects)
	{
		if(EffectClass)
		{
			FGameplayEffectContextHandle EffectContext = MakeEffectContext();
			EffectContext.AddSourceObject(this);
			FGameplayEffectSpecHandle NewHandle = MakeOutgoingSpec(EffectClass, 1, EffectContext);
			if(NewHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
			}
		}
	}
}
