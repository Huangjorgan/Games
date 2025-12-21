
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "MC_Character.generated.h"

/**
* MC_Character
* 本地玩家角色类
*/
UCLASS()
class GAMECORE_API AMC_Character : public ACharacter
{
	GENERATED_BODY()

public:
	AMC_Character();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	// 处理用于前后移动的输入。
	UFUNCTION()
	void MoveForward(float value);

	// 处理用于左右移动的输入。
	UFUNCTION()
	void MoveRight(float value);

	// 按下键时，设置跳跃标记。
	UFUNCTION()
	void StartJump();

	// 释放键时，清除跳跃标记。
	UFUNCTION()
	void StopJump();

	//第一人称摄像机
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* FPSCameraComponent;

	// 第一人称网格体（手臂），仅对所属玩家可见。
	UPROPERTY(VisibleAnywhere, Category = Mesh)
	USkeletalMeshComponent* FPSMesh;
};
