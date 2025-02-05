#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyCharacter.generated.h"

class USpringArmComponent;
class UArrowComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UCapsuleComponent;
struct FInputActionValue;

UCLASS()
class MOVECHARACTER_API AMyCharacter : public APawn
{
	GENERATED_BODY()

public:
	AMyCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Casule");
	UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow");
	UArrowComponent* Arrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh");
	USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera");
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera");
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	class UBoxComponent* JumpTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	class UBoxComponent* HeadTrigger;

protected:
	// ī�޶� ȸ���ӵ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyCharacter|Properties")
	float RotationSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyCharacter|Properties")
	float moveSpeed = 100.0f;

	// ������ ���� �����ϴ� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyCharacter|Properties")
	float JumpPower = 300.0f;

	bool bIsFalling;

	// ���� ������ �ӵ��� ��Ÿ���� ���� (�ַ� Z�� �ӵ� ���)
	FVector Velocity;

	float cameraPitch;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FInputActionValue& value);

	UFUNCTION()
	void StartJump(const FInputActionValue& value);

	UFUNCTION()
	void StopJump(const FInputActionValue& value);

	UFUNCTION()
	void Look(const FInputActionValue& value);

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnJumpTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnHeadTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
