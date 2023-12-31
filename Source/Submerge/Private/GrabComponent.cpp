// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Switch.h"
#include "UI/InteractWidget.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/Soundbase.h"

// Sets default values for this component's properties
UGrabComponent::UGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrabComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get Capsule
	Capsule = GetOwner()->FindComponentByClass<UCapsuleComponent>();
	if (Capsule)
	{
		// Do nothing
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Capsule found"));
	}

	// Get Camera
	Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
	if (Camera)
	{
		// Do nothing
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Camera found"));
	}

	// Get Physics Handle
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle)
	{
		// Do nothing
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Physics Handle found"));
	}
}

// Called every frame
void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// raycast here - save reference - check if reference has changed
	// use grab functionality to instead grab the saved reference object and don't call the raycast in that function.
	
	// Setup for linetrace
	FVector Start = Camera->GetComponentLocation();
	FVector End = (Camera->GetComponentRotation().Vector() * InteractionDistance) + Start;
	FCollisionQueryParams CollisionParams(FName(TEXT("")), false, GetOwner());

	// Check for pickup objects
	FHitResult HitResult;
	bool IsHit = GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End,
		ECC_PhysicsBody, CollisionParams);

	// Check for objects in front of pickup objects
	FHitResult ObstacleHit;
	bool ObHit = GetWorld()->LineTraceSingleByObjectType(ObstacleHit, Start, End,
		ECC_WorldStatic, CollisionParams);
	
	// Check for obstructing objects
	if (!ObHit)
	{
		// Check for pickup object hit
		if (IsHit)
		{
			AddInteractWidget();
		}
		else
		{
			// DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f);

			// move this widget removal to a conditional where it should consistently be removed.
			if (InteractWidget != nullptr)
			{
				InteractWidget->RemoveFromParent();
			}
		}
	}
	// rearrange these conditionals - when statement? Switch case?
	else
	{
		TObjectPtr<ASwitch> Switch = Cast<ASwitch>(ObstacleHit.GetActor());
		if (Switch)
		{
			UE_LOG(LogTemp, Warning, TEXT("Switch."));

			AddInteractWidget();
		}
	}

	if (HoldingObject)
	{
		if (Capsule)
		{
			FVector HoldStart = Capsule->GetComponentLocation();
			FVector HoldEnd = (Capsule->GetComponentRotation().Vector() * HoldDistance) + HoldStart;

			HoldEnd.Z += HoldHeight;

			if (PhysicsHandle->GrabbedComponent)
			{
				PhysicsHandle->SetTargetLocation(HoldEnd);
			}
		}
	}

}

void UGrabComponent::Grab()
{
	if (HoldingObject)
	{
		PhysicsHandle->GrabbedComponent->SetCollisionProfileName(FName("PhysicsActor"));
		PhysicsHandle->ReleaseComponent();
		HoldingObject = false;
		
		FVector Start = Camera->GetComponentLocation();
		FVector End = (Camera->GetComponentRotation().Vector() * InteractionDistance) + Start;
		UGameplayStatics::SpawnSoundAtLocation(this, PutDownSound, End);
	}
	else
	{
		if (Camera)
		{
			PickUpObject();
		}
	}
}

void UGrabComponent::PickUpObject()
{
	// Setup for linetrace
	FVector Start = Camera->GetComponentLocation();
	FVector End = (Camera->GetComponentRotation().Vector() * InteractionDistance) + Start;
	FCollisionQueryParams CollisionParams(FName(TEXT("")), false, GetOwner());

	// Check for pickup objects
	FHitResult HitResult;
	bool IsHit = GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End,
		ECC_PhysicsBody, CollisionParams);

	// Check for objects in front of pickup objects
	FHitResult ObstacleHit;
	bool ObHit = GetWorld()->LineTraceSingleByObjectType(ObstacleHit, Start, End,
		ECC_WorldStatic, CollisionParams);
	
	// Check for obstructing objects
	if (!ObHit)
	{
		// Check for pickup object hit
		if (IsHit)
		{
			CheckIfObjectIsBelow(HitResult, Start, End);
			UGameplayStatics::SpawnSoundAtLocation(this, PickUpSound, FVector(End));
		}
		else
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f);
		}
	}
	else
	{
		TObjectPtr<ASwitch> Switch = Cast<ASwitch>(ObstacleHit.GetActor());
		if (Switch)
		{
			Switch->TurnOn();

			UE_LOG(LogTemp, Warning, TEXT("Switch."));
		}
	}
}

// No longer checks if there is an object below
void UGrabComponent::CheckIfObjectIsBelow(const FHitResult& HitResult, const FVector& Start, const FVector& End)
{
	AActor* Actor = HitResult.GetActor();
	UE_LOG(LogTemp, Error, TEXT("Raycast hit: %s"), *FString(Actor->GetName()));
	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.f);

	UPrimitiveComponent* ComponentToGrab = HitResult.GetComponent();

	if (ComponentToGrab && (ComponentToGrab != CollideComp))
	{
		ComponentToGrab->AddImpulse(FVector(0.f, 0.f, 0.1f), FName("NAME_None"), true);

		//Actor->SetActorEnableCollision(false);
		ComponentToGrab->SetCollisionProfileName(FName("PickedUp"));

		PhysicsHandle->GrabComponentAtLocationWithRotation(ComponentToGrab, FName("NAME_None"), HitResult.Location, ComponentToGrab->GetComponentRotation());

		HoldingObject = true;
	}
}

void UGrabComponent::AddInteractWidget()
{
	if (InteractWidget == nullptr)
	{
		if (IsValid(WidgetClass))
		{
			InteractWidget = Cast<UInteractWidget>(CreateWidget(GetWorld(), WidgetClass));
		}
	}
			
	if (InteractWidget != nullptr)
	{
		InteractWidget->AddToViewport();
	}
}

// Not relevant atm
// Delete this
TArray<FHitResult> UGrabComponent::CheckCollisionUnderneath() const
{
	TArray<FHitResult> HitResult;

	// Check right underneath the capsule
	FCollisionQueryParams CollisionParams(FName(TEXT("")), false, GetOwner());


	for (size_t i = 0; i < Iterations; i++)
	{
		FVector Start = Capsule->GetComponentLocation();
		Start += Capsule->GetForwardVector() * (i/(Iterations-1)) * (Capsule->GetScaledCapsuleRadius() + 20);
		FVector End = (Capsule->GetUpVector()) + Start;
		End.Z *= -1;
		FHitResult Result;
		bool Hit = GetWorld()->LineTraceSingleByObjectType(Result, Start, End,
			ECC_PhysicsBody, CollisionParams);

		HitResult.Emplace(Result);

		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 5.f);
	}

	return HitResult;
}

