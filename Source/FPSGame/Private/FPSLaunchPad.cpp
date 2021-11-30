// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSLaunchPad.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFPSLaunchPad::AFPSLaunchPad()
{
 	OverlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
	OverlapComp->SetBoxExtent(FVector(75,75,50));
	RootComponent = OverlapComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));	
	MeshComp->SetupAttachment(RootComponent);

	//Bind to Event
	OverlapComp->OnComponentBeginOverlap.AddDynamic(this, &AFPSLaunchPad::OverlapLaunchPad);

	LaunchStrength = 1500;
	LaunchPitchAngle = 35.0f;


}

void AFPSLaunchPad::OverlapLaunchPad(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
		FRotator LaunchDirection = GetActorRotation();
		LaunchDirection.Pitch += LaunchPitchAngle;
		FVector LaunchVelocity = LaunchDirection.Vector() * LaunchStrength;

		ACharacter* OtherCharacter = Cast<ACharacter>(OtherActor);
		if (OtherCharacter)
		{
			//launch Player || Both booleans give consistent laucnh velo by ignoring current player velo
			OtherCharacter->LaunchCharacter(LaunchVelocity,true,true);

			//Spawn FX
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ActivateLaunchPadEffect,GetActorLocation());

		}

		else if(OtherComp && OtherComp->IsSimulatingPhysics())
		{
			OtherComp->AddImpulse(LaunchVelocity,NAME_None,true);

			//SpawnFx
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ActivateLaunchPadEffect,GetActorLocation());
		}
		
}

