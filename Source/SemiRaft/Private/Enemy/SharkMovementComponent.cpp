// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/SharkMovementComponent.h"


FVector GetQuadraticBezierDirection(
	const FVector& P0, 
	const FVector& P1, 
	const FVector& P2,
	const float Alpha
)
{
	const float T = FMath::Clamp(Alpha, 0.0f, 1.0f);

	const FVector Tangent =
		2.0f * (1.0f - T) * (P1 - P0) +
		2.0f * T * (P2 - P1);

	return Tangent.GetSafeNormal();
}

void USharkMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
}

void USharkMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	
	
	// ShouldSkipUpdate가 true면 이동하면 안 된다.
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	if (!UpdatedComponent)
	{
		return;
	}

	if (TargetPos != FVector::ZeroVector)
	{
		ApplyDeceleration(DeltaTime);
		MoveShark(DeltaTime);
		return;
	}

	const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	const FVector TargetLocation = TargetPos;

	UpdateSharkVelocity(
		CurrentLocation,
		TargetLocation,
		DeltaTime
	);

	MoveShark(DeltaTime);
}

void USharkMovementComponent::UpdateSharkVelocity(
	const FVector& CurrentLocation,
	const FVector& TargetLocation,
	float DeltaTime
)
{
	const FVector ToTarget = TargetLocation - CurrentLocation;
	const float DistanceToTarget = ToTarget.Size();

	if (DistanceToTarget <= AcceptanceRadius)
	{
		ApplyDeceleration(DeltaTime);
		return;
	}

	if (ToTarget.IsNearlyZero())
	{
		ApplyDeceleration(DeltaTime);
		return;
	}

	const FVector DesiredDirection = ToTarget.GetSafeNormal();

	FVector Forward = CurrentVelocity.GetSafeNormal();

	if (Forward.IsNearlyZero())
	{
		Forward = UpdatedComponent->GetForwardVector().GetSafeNormal();
	}

	const float CurrentSpeed = CurrentVelocity.Size();

	const float ForwardDot = FVector::DotProduct(
		Forward,
		DesiredDirection
	);

	const float AccelScale = FMath::Clamp(
		ForwardDot,
		0.0f,
		1.0f
	);

	// 목표 근처에서는 목표 속력을 낮춤.
	const float SpeedScale = FMath::Clamp(
		DistanceToTarget / SlowDownRadius,
		0.0f,
		1.0f
	);

	const float DesiredSpeed = FMath::Lerp(
		MinCruiseSpeed,
		MaxSpeed,
		SpeedScale
	);

	// 현재 속력에서 가능한 최소 선회 반경.
	const float CurrentTurnRadius =
		(CurrentSpeed * CurrentSpeed) / FMath::Max(MaxTurnAcceleration, 1.0f);

	// 목표가 현재 선회 반경 안쪽에 있으면 계속 빙빙 돌 위험이 큼.
	const bool bCannotTurnIntoTarget =
		DistanceToTarget < CurrentTurnRadius * 0.8f;

	float NewSpeed = CurrentSpeed;

	if (bCannotTurnIntoTarget)
	{
		// 궤도 포획 탈출용 강제 감속.
		NewSpeed -= Deceleration * 1.5f * DeltaTime;
	}
	else if (CurrentSpeed < DesiredSpeed)
	{
		NewSpeed += ForwardAcceleration * AccelScale * DeltaTime;
	}
	else
	{
		NewSpeed -= Deceleration * DeltaTime;
	}

	NewSpeed = FMath::Clamp(
		NewSpeed,
		MinCruiseSpeed,
		MaxSpeed
	);

	const float SafeSpeed = FMath::Max(NewSpeed, 1.0f);

	const float MaxTurnRadiansThisTick =
		(MaxTurnAcceleration / SafeSpeed) * DeltaTime;

	const FVector NewDirection = RotateDirectionToward(
		Forward,
		DesiredDirection,
		MaxTurnRadiansThisTick
	);

	CurrentVelocity = NewDirection * NewSpeed;
}

FVector USharkMovementComponent::RotateDirectionToward(const FVector& CurrentDirection, const FVector& DesiredDirection,
	const float MaxRadians)
{
	const FVector Current = CurrentDirection.GetSafeNormal();
	const FVector Desired = DesiredDirection.GetSafeNormal();

	const float Dot = FMath::Clamp(
		FVector::DotProduct(Current, Desired),
		-1.0f,
		1.0f
	);

	const float Angle = FMath::Acos(Dot);

	if (Angle <= KINDA_SMALL_NUMBER)
	{
		return Desired;
	}

	const float ClampedAngle = FMath::Min(Angle, MaxRadians);

	FVector Axis = FVector::CrossProduct(Current, Desired);

	if (Axis.IsNearlyZero())
	{
		Axis = FVector::UpVector;
	}
	else
	{
		Axis.Normalize();
	}

	return Current.RotateAngleAxis(
		FMath::RadiansToDegrees(ClampedAngle),
		Axis
	).GetSafeNormal();
}

void USharkMovementComponent::MoveShark(const float DeltaTime)
{
	if (!UpdatedComponent)
	{
		return;
	}

	if (CurrentVelocity.IsNearlyZero())
	{
		return;
	}

	const FVector DeltaMove = CurrentVelocity * DeltaTime;
	const FRotator NewRotation = CurrentVelocity.GetSafeNormal().Rotation();

	FHitResult Hit;

	SafeMoveUpdatedComponent(
		DeltaMove,
		NewRotation,
		true,
		Hit
	);

	if (Hit.IsValidBlockingHit())
	{
		SlideAlongSurface(
			DeltaMove,
			1.0f - Hit.Time,
			Hit.Normal,
			Hit
		);
	}
}

void USharkMovementComponent::ApplyDeceleration(const float DeltaTime)
{
	const float CurrentSpeed = CurrentVelocity.Size();

	if (CurrentSpeed <= KINDA_SMALL_NUMBER)
	{
		CurrentVelocity = FVector::ZeroVector;
		return;
	}

	const float NewSpeed = FMath::Max(
		0.0f,
		CurrentSpeed - Deceleration * DeltaTime
	);

	CurrentVelocity = CurrentVelocity.GetSafeNormal() * NewSpeed;
}

void USharkMovementComponent::SetTargetPos(FVector NewTargetPos)
{
	TargetPos = NewTargetPos;
}

