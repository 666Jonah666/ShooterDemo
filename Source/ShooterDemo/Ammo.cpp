// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

AAmmo::AAmmo() {
	//construct the ammo mesh component and set it as the root
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>("AmmoMesh");
	SetRootComponent(AmmoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());
	
}

void AAmmo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void AAmmo::BeginPlay() {
	Super::BeginPlay();
}

void AAmmo::SetItemProperties(EItemState State) {
	Super::SetItemProperties(State);

	switch (State) {

	case EItemState::EIS_Pickup:
		//set mesh properties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;
	case EItemState::EIS_EquipInterping:
		//set mesh properties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_PickedUp: break;
	case EItemState::EIS_Equipped:
		//set mesh properties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		//Set mesh properties
		AmmoMesh->SetSimulatePhysics(true);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AmmoMesh->SetEnableGravity(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		AmmoMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

		break;
	case EItemState::EIS_MAX: break;
	default: ;
	}
}
