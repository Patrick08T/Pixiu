// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/CPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
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

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f); // ...at this rotation rate
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
			InputSubsystem->AddMappingContext(GamePlayerCharacterMappingContext, 0); // 如果有默认映射上下文，可以在此添加
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
