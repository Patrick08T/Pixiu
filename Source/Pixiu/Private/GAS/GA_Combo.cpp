// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GameFramework/Pawn.h"

UGA_Combo::UGA_Combo()
{
	AbilityTags.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ComboMontage);
		PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitForNextComboEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboChangedEventTag(), nullptr, false, false);
		WaitForNextComboEvent->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangedEventReceived);
		WaitForNextComboEvent->ReadyForActivation();
	}

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetingEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboTargetEventTag());
		WaitTargetingEventTask->EventReceived.AddDynamic(this, &UGA_Combo::DoDamage);
		WaitTargetingEventTask->ReadyForActivation();
	}
	SetupWaitComboInputPress();
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.change");
}

FGameplayTag UGA_Combo::GetComboAbilityEndTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.change.end");
}

FGameplayTag UGA_Combo::GetComboTargetEventTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.damage");
}

void UGA_Combo::SetupWaitComboInputPress()
{
    UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
    WaitInputPress->OnPress.AddDynamic(this, &UGA_Combo::HandleInputPress);
    WaitInputPress->ReadyForActivation();
}

void UGA_Combo::HandleInputPress(float TimeWaited)
{
	SetupWaitComboInputPress();
	TryCommitCombo();
}

void UGA_Combo::TryCommitCombo()
{
	if (NextComboName == NAME_None)
	{
		return;
	}

	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (!OwnerAnimInst)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Next Combo is %s %s"), *OwnerAnimInst->Montage_GetCurrentSection(ComboMontage).ToString(), *NextComboName.ToString());
	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(ComboMontage), NextComboName, ComboMontage);
}

TSubclassOf<UGameplayEffect> UGA_Combo::GetDamageEffectForCurrentCombo() const
{
	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
	if (OwnerAnimInstance)
	{
		FName CurrentSectionName = OwnerAnimInstance->Montage_GetCurrentSection(ComboMontage);
		const TSubclassOf<UGameplayEffect>* FoundEffectPtr = DamageEffectMap.Find(CurrentSectionName);
		if (FoundEffectPtr)
		{
			return *FoundEffectPtr;
		}
	}
	return DefaultDamageEffect;
}

void UGA_Combo::ComboChangedEventReceived(FGameplayEventData Payload)
{
	FGameplayTag EventTag = Payload.EventTag;

	if (EventTag == GetComboAbilityEndTag())
	{
		NextComboName = NAME_None;
		UE_LOG(LogTemp, Warning, TEXT("Next Combo is cleared"));
		return;
	}
	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	NextComboName = TagNames.Last();

	UE_LOG(LogTemp, Warning, TEXT("Next Combo Name: %s"), *NextComboName.ToString());
}

void UGA_Combo::DoDamage(FGameplayEventData Payload)
{
	TArray<FHitResult> HitResults = GetHitResultFromSweepLocationTargetData(Payload.TargetData, TaretSweepSphereRadius, false, true);

	for (const FHitResult& HitResult : HitResults)
	{
		TSubclassOf<UGameplayEffect> GameplayEffect = GetDamageEffectForCurrentCombo();
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, GetAbilityLevel(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()));
		
		FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
		EffectContext.AddHitResult(HitResult);

		EffectSpecHandle.Data->SetContext(EffectContext);

		ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
	}
}
