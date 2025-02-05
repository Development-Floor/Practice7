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

	//생성자 위치에서 Capsule의 변수를 설정해줌
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetCapsuleHalfHeight(20.f);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);  // 충돌 활성화
	Capsule->SetCollisionResponseToAllChannels(ECR_Block);  // 모든 채널에 대해 충돌 활성화
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

	// 점프 트리거 박스 생성
	JumpTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("JumpTrigger"));
	JumpTrigger->SetupAttachment(RootComponent);
	JumpTrigger->SetRelativeScale3D(FVector(0.25, 0.25, 0.25));
	JumpTrigger->SetRelativeLocation(FVector(0, 0, -80));
	JumpTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	JumpTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	JumpTrigger->SetCollisionResponseToAllChannels(ECR_Overlap);

	// 트리거 박스의 OnComponentBeginOverlap 이벤트 바인딩
	JumpTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnJumpTriggerBeginOverlap);

	// 헤드 트리거 박스 생성
	HeadTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadTrigger"));
	HeadTrigger->SetupAttachment(RootComponent);
	HeadTrigger->SetRelativeScale3D(FVector(0.25, 0.25, 0.25));
	HeadTrigger->SetRelativeLocation(FVector(0, 0, 90));
	HeadTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HeadTrigger->SetCollisionResponseToAllChannels(ECR_Overlap);

	// 트리거 박스의 OnComponentBeginOverlap 이벤트 바인딩
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
	// 다른 액터와 충돌 시 처리
	if (OtherActor && OtherActor != this)
	{
		if (bIsFalling)
		{
			bIsFalling = false;  // 점프 종료
			Velocity.Z = 0;  // 수직 속도 초기화
			FVector NewLocation = GetActorLocation();
			NewLocation.Z = SweepResult.ImpactPoint.Z;  // 착지 위치의 Z 값으로 설정
			SetActorLocation(NewLocation);
		}
	}
}

// 천장에 박을 시
void AMyCharacter::OnHeadTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 다른 액터와 충돌 시 처리
	if (OtherActor && OtherActor != this)
	{
		if (bIsFalling)
		{
			Velocity.Z = 0;  // 수직 속도 초기화
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
	// 더블 점프 방지
	if (!bIsFalling)
	{
		// 점프를 시작하려면 수직 속도에 힘을 가해줍니다.
		Velocity.Z = JumpPower;
		bIsFalling = true;
	}
}

void AMyCharacter::StopJump(const FInputActionValue& value)
{
	// 점프를 멈추는 대신, 중력이 계속 적용
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

	Velocity.Z -= 980 * DeltaTime;  // 중력 적용

	// 이동할 위치 계산
	FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;

	// 충돌을 고려하여 이동
	FHitResult HitResult;
	SetActorLocation(NewLocation, true, &HitResult);

	// 착지 처리
	if (HitResult.bBlockingHit)
	{
		if (HitResult.Normal.Z > 0.7f)  // Z 축 방향으로 바닥과 충돌했다면
		{
			bIsFalling = false;  // 점프 종료
			Velocity.Z = 0;  // 수직 속도 초기화
		}
	}
}
