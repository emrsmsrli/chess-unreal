// Copyright 2018 Emre Simsirli

#include "ChessPiece.h"
#include "Side.h"
#include "DestructibleComponent.h"

AChessPiece::AChessPiece()
{
    PrimaryActorTick.bCanEverTick = true;

    /*Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("mesh"));
    RootComponent = Mesh;*/
    RootComponent = GetDestructibleComponent();
    //GetDestructibleComponent()->SetSimulatePhysics(true);
    Side = ESide::white;

    //void OnMyActorFracture(const FVector& HitPoint, const FVector& HitDirection);
    //OnActorFracture.AddUniqueDynamic(this, &AChessPiece::OnMyActorFracture);
}

void AChessPiece::BeginPlay()
{
    AActor::BeginPlay();
}

void AChessPiece::Tick(const float DeltaTime)
{
    AActor::Tick(DeltaTime);
}
