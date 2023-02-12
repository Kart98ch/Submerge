// Fill out your copyright notice in the Description page of Project Settings.


#include "TriggerReactComponent.h"
#include "ActorMovementComponent.h"
#include "TriggerActor.h"
#include "TriggerInteractionComponent.h"

// Sets default values for this component's properties
UTriggerReactComponent::UTriggerReactComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ActorMoveComponent = CreateDefaultSubobject<UActorMovementComponent>(FName("ActorMoveComponent"));

}


// Called when the game starts
void UTriggerReactComponent::BeginPlay()
{
	Super::BeginPlay();

	TriggerCompEventSource = TriggerActorEventSource->FindComponentByClass<UTriggerInteractionComponent>();

	if (TriggerCompEventSource)
	{
		TriggerCompEventSource->OnActivation().AddUObject(this, &UTriggerReactComponent::ActivateActor);
		TriggerCompEventSource->OnDeactivation().AddUObject(this, &UTriggerReactComponent::DeactivateActor);
	}
}

void UTriggerReactComponent::ActivateActor()
{
	MoveState = true;
}

void UTriggerReactComponent::DeactivateActor()
{
	MoveState = false;
}


// Called every frame
void UTriggerReactComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TriggerActorEventSource)
	{
		if (MoveState)
		{
			ActorMoveComponent->Move();
		}
		else
		{
			ActorMoveComponent->MoveBack();
		}
	}
}

