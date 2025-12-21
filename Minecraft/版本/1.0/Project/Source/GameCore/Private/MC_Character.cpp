
#include "MC_Character.h"

AMC_Character::AMC_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	//创建第一人称摄像机组件
	FPSCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	check(FPSCameraComponent != nullptr);

	//将摄像机组件附加到胶囊体组件
	FPSCameraComponent->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));

	// 将摄像机置于略高于眼睛上方的位置
	FPSCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f + BaseEyeHeight));

	// 启用Pawn控制摄像机旋转。
	FPSCameraComponent->bUsePawnControlRotation = true;

	// 为所属玩家创建第一人称网格体组件。
	FPSMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));

	// 只有所属玩家可以看见此网格体。
	FPSMesh->SetOnlyOwnerSee(true);

	// 将FPS网格体附加到FPS摄像机。
	FPSMesh->SetupAttachment(FPSCameraComponent);

	// 禁用某些环境阴影以便实现只有一个网格体的感觉。
	FPSMesh->bCastDynamicShadow = false;
	FPSMesh->CastShadow = false;

	// 所属玩家看不到常规（第三人称）全身网格体。
	GetMesh()->SetOwnerNoSee(true);
}

void AMC_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMC_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMC_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 设置"移动"绑定。
	PlayerInputComponent->BindAxis("MoveForward", this, &AMC_Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMC_Character::MoveRight);

	//设置"观看"绑定
	PlayerInputComponent->BindAxis("Turn", this, &AMC_Character::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AMC_Character::AddControllerPitchInput);

	//设置"操作"绑定
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMC_Character::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMC_Character::StopJump);
}


void AMC_Character::MoveForward(float value)
{
	// 找出"前进"方向，并记录玩家想向该方向移动。
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, value);
}

void AMC_Character::MoveRight(float value)
{
	// 找出"右侧"方向，并记录玩家想向该方向移动。
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, value);
}

void AMC_Character::StartJump()
{
	bPressedJump = true;
}

void AMC_Character::StopJump()
{
	bPressedJump = false;
}