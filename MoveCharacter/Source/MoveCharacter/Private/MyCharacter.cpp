#include "MyCharacter.h"
#include "EnhancedInputComponent.h"
#include "MyPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bIsFalling = false;

	cameraPitch = 0;

	//������ ��ġ���� Capsule�� ������ ��������
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetCapsuleHalfHeight(20.f);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);  // �浹 Ȱ��ȭ
	Capsule->SetCollisionResponseToAllChannels(ECR_Block);  // ��� ä�ο� ���� �浹 Ȱ��ȭ
	SetRootComponent(Capsule);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(GetRootComponent());

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	// ���� Ʈ���� �ڽ� ����
	JumpTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("JumpTrigger"));
	JumpTrigger->SetupAttachment(RootComponent);
	JumpTrigger->SetRelativeScale3D(FVector(0.25, 0.25, 0.25));
	JumpTrigger->SetRelativeLocation(FVector(0, 0, -80));
	JumpTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	JumpTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	JumpTrigger->SetCollisionResponseToAllChannels(ECR_Overlap);

	// Ʈ���� �ڽ��� OnComponentBeginOverlap �̺�Ʈ ���ε�
	JumpTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnJumpTriggerBeginOverlap);

	// ��� Ʈ���� �ڽ� ����
	HeadTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadTrigger"));
	HeadTrigger->SetupAttachment(RootComponent);
	HeadTrigger->SetRelativeScale3D(FVector(0.25, 0.25, 0.25));
	HeadTrigger->SetRelativeLocation(FVector(0, 0, 90));
	HeadTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HeadTrigger->SetCollisionResponseToAllChannels(ECR_Overlap);

	// Ʈ���� �ڽ��� OnComponentBeginOverlap �̺�Ʈ ���ε�
	HeadTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnHeadTriggerBeginOverlap);
}

void AMyCharacter::OnJumpTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// �ٸ� ���Ϳ� �浹 �� ó��
	if (OtherActor && OtherActor != this)
	{
		if (bIsFalling)
		{
			bIsFalling = false;  // ���� ����
			Velocity.Z = 0;  // ���� �ӵ� �ʱ�ȭ
			FVector NewLocation = GetActorLocation();
			NewLocation.Z = SweepResult.ImpactPoint.Z;  // ���� ��ġ�� Z ������ ����
			SetActorLocation(NewLocation);
		}
	}
}

// õ�忡 ���� ��
void AMyCharacter::OnHeadTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// �ٸ� ���Ϳ� �浹 �� ó��
	if (OtherActor && OtherActor != this)
	{
		if (bIsFalling)
		{
			Velocity.Z = 0;  // ���� �ӵ� �ʱ�ȭ
		}
	}
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&AMyCharacter::Move
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&AMyCharacter::StartJump
				);

				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Completed,
					this,
					&AMyCharacter::StopJump
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&AMyCharacter::Look
				);
			}
		}
	}
}

void AMyCharacter::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector2D MoveInput = value.Get<FVector2D>();

	FVector toMove;

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		toMove = GetActorForwardVector() * MoveInput.X;
		AddActorWorldOffset(toMove * moveSpeed);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		toMove = GetActorRightVector() * MoveInput.Y;
		AddActorWorldOffset(toMove * moveSpeed);
	}
}

void AMyCharacter::StartJump(const FInputActionValue& value)
{
	// ���� ���� ����
	if (!bIsFalling)
	{
		// ������ �����Ϸ��� ���� �ӵ��� ���� �����ݴϴ�.
		Velocity.Z = JumpPower;
		bIsFalling = true;
	}
}

void AMyCharacter::StopJump(const FInputActionValue& value)
{
	// ������ ���ߴ� ���, �߷��� ��� ����
}

void AMyCharacter::Look(const FInputActionValue& value)
{
	if (!Controller)
	{
		return;
	}

	FVector2D LookInput = value.Get<FVector2D>() * RotationSpeed;

	if (!FMath::IsNearlyZero(LookInput.X))
	{
		AddActorLocalRotation(FRotator(0, LookInput.X, 0), false);
	}

	if (!FMath::IsNearlyZero(LookInput.Y))
	{
		if (cameraPitch + LookInput.Y > 90)
		{
			return;
		}

		if (cameraPitch + LookInput.Y < -90)
		{
			return;
		}

		cameraPitch += LookInput.Y;

		SpringArmComp->AddLocalRotation(FRotator(LookInput.Y, 0, 0), false);
	}
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Velocity.Z -= 980 * DeltaTime;  // �߷� ����

	// �̵��� ��ġ ���
	FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;

	// �浹�� ����Ͽ� �̵�
	FHitResult HitResult;
	SetActorLocation(NewLocation, true, &HitResult);

	// ���� ó��
	if (HitResult.bBlockingHit)
	{
		if (HitResult.Normal.Z > 0.7f)  // Z �� �������� �ٴڰ� �浹�ߴٸ�
		{
			bIsFalling = false;  // ���� ����
			Velocity.Z = 0;  // ���� �ӵ� �ʱ�ȭ
		}
	}
}
