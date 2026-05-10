// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/CPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
ACPlayerCharacter::ACPlayerCharacter()
{
	// Create SpringArm and attach to root (capsule)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create Camera and attach to spring arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false; // Character rotation is controlled by camera/controller input
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}

void ACPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	APlayerController* OwningPlayerController = GetController<APlayerController>();
	if (OwningPlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if(InputSubsystem)
		{
			InputSubsystem->RemoveMappingContext(GamePlayerCharacterMappingContext);
			InputSubsystem->AddMappingContext(GamePlayerCharacterMappingContext, 0); // �����Ĭ��ӳ�������ģ������ڴ�����
		}
	}
}

void ACPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleLookInput);
		EnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleMoveInput);
		for(const TPair<ECAbilityInputID,UInputAction*>& Pair : GameplayAbilityInputActions)
		{
			EnhancedInputComponent->BindAction(Pair.Value, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleAbilityInput, Pair.Key);
		}
	}
}

void ACPlayerCharacter::HandleLookInput(const FInputActionValue& Value)
{
	FVector2D LookInputValue = Value.Get<FVector2D>();
	AddControllerYawInput(LookInputValue.X);
	AddControllerPitchInput(-LookInputValue.Y);
}

void ACPlayerCharacter::HandleMoveInput(const FInputActionValue& Value)
{
	FVector2D MoveInputValue = Value.Get<FVector2D>();
	MoveInputValue.Normalize();
	
	AddMovementInput(GetMoveFwdDir() * MoveInputValue.Y + GetLookRightDir() * MoveInputValue.X);
}

void ACPlayerCharacter::HandleAbilityInput(const FInputActionValue& Value, ECAbilityInputID InputID)
{
	bool bPressed = Value.Get<bool>();
	if (bPressed)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->AbilityLocalInputPressed((int32)InputID);
		}
	}
	else
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->AbilityLocalInputReleased((int32)InputID);
		}
	}
}

void ACPlayerCharacter::OnDead()
{
	auto* PlayerController = GetController<APlayerController>();
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}

void ACPlayerCharacter::OnRespawn()
{
	auto* PlayerController = GetController<APlayerController>();
	if (PlayerController)
	{
		EnableInput(PlayerController);
	}
}

FVector ACPlayerCharacter::GetLookRightDir() const
{
	return Camera->GetRightVector();
}

FVector ACPlayerCharacter::GetLookFwdDir() const
{
	return Camera->GetForwardVector();
}

FVector ACPlayerCharacter::GetMoveFwdDir() const
{
	return FVector::CrossProduct(GetLookRightDir(), FVector::UpVector);
}
