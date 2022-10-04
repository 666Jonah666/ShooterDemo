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
