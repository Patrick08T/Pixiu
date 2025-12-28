// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/CPlayerCharacter.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ACPlayerCharacter::ACPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create SpringArm and attach to root (capsule)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	// 把弹簧臂上移到胶囊上方并略微向下俯视
	// 这些值可在编辑器里根据角色尺寸微调
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 50.f));    // 把臂根部抬高
	SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));  // 轻微俯视角度

	SpringArm->TargetArmLength = 300.f;                 // 默认距离
	SpringArm->SocketOffset = FVector(0.f, 20.f, 0.f);   // 相机在臂末端稍微向右偏移（避免正中遮挡）
	SpringArm->bUsePawnControlRotation = true;          // 随控制器旋转
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.f;
	SpringArm->bDoCollisionTest = true;                 // 碰撞检测，避免穿模

	// 让角色的胶囊忽略弹簧臂所用的探针通道（通常为 ECC_Camera）
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	// Create Camera and attach to spring arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;            // 由弹簧臂控制旋转
	Camera->SetRelativeLocation(FVector::ZeroVector);

	// 可选：设置视场（根据需求）
	// Camera->FieldOfView = 90.f;

	// 控制器是否影响角色朝向
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void ACPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	// 绑定镜头旋转轴（确保在 Project Settings -> Input 中存在这些 Axis 映射）
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ACPlayerCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACPlayerCharacter::LookUp);
}

void ACPlayerCharacter::Turn(float Value)
{
	if (Value != 0.0f)
	{
		AddControllerYawInput(Value);
	}
}

void ACPlayerCharacter::LookUp(float Value)
{
	if (Value != 0.0f)
	{
		AddControllerPitchInput(Value);
	}
}

