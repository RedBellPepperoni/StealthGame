// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "FPSGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnNoiseHeard);

	GuardState = EAIState::Idle;
}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();

	OriginalRotation = GetActorRotation();

	if(bPatrol)
	{
		MoveToNextPoint();
	}
	
}


void AFPSAIGuard::OnPawnSeen(APawn* SeenPawn)
{
	if(SeenPawn == nullptr)
	{
		return;
	}

	// Temporary Check for Pawn sensing the player 
	DrawDebugSphere(GetWorld(),SeenPawn->GetActorLocation(), 32.0f, 12, FColor::Red, false,10.0f);


	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
		if(GM)
		{
			GM->CompleteMission(SeenPawn, false);
		}

		SetGuardState(EAIState::Alerted);

		//Stop moving
		AController* Controller = GetController();
		if(Controller)
		{
			Controller->StopMovement();
		}
}

void AFPSAIGuard::OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    if(GuardState == EAIState::Alerted)
	{
		return;
	}


	DrawDebugSphere(GetWorld(), Location, 32.0f, 12, FColor::Green, false,10.0f);

	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.0f;
	NewLookAt.Roll = 0.0f;

	SetActorRotation(NewLookAt);

	

    GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);

    
	SetGuardState(EAIState::Suspicious);

	AController* Controller = GetController();

	if(Controller)
	{
		Controller->StopMovement();
	}
	
}

void AFPSAIGuard::ResetOrientation()
{


	if(GuardState == EAIState::Alerted)
	{
		return;
	}

	
	SetActorRotation(OriginalRotation);


	SetGuardState(EAIState::Idle);

	if(bPatrol)
	{
		MoveToNextPoint();
	}
}


void AFPSAIGuard::OnRep_GuardState()
{
	OnStateChanged(GuardState);
}


void AFPSAIGuard::SetGuardState(EAIState NewState)
{

	if(GuardState == NewState)
	{
		return;
	}

	GuardState = NewState;

	OnRep_GuardState();

	

}

void AFPSAIGuard::MoveToNextPoint()
{
	//Assign next point
	if(CurrentPatrolPoint == nullptr || CurrentPatrolPoint == SecondPatrolPoint)
	{
		CurrentPatrolPoint = FirstPatrolPoint;
	}
	else
	{
		CurrentPatrolPoint = SecondPatrolPoint;
	}

	UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), CurrentPatrolPoint->GetActorLocation());
}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Patrol Check
	if(CurrentPatrolPoint)
	{
		FVector Delta = GetActorLocation() - CurrentPatrolPoint->GetActorLocation();
		float DistanceToGoal = Delta.Size();

		//Check if we are within x units of our goal - if yes pick a new point
		if(DistanceToGoal < 100)
		{
			MoveToNextPoint();
		}
	}

}

void AFPSAIGuard::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSAIGuard, GuardState);
	
}



