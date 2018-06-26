// Fill out your copyright notice in the Description page of Project Settings.

#include "NewActorComponent.h"
#include "ChessEngine.h"
#include "Board.h"

#define LOG(s, ...) UE_LOG(LogTemp, Log, TEXT(s), __VA_ARGS__)

// Sets default values for this component's properties
UNewActorComponent::UNewActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UNewActorComponent::BeginPlay()
{
	Super::BeginPlay();
    engine::init();

    /*engine::bitboard b;
    b.set_sq(engine::transition::sq64(engine::square::d2));
    b.set_sq(engine::transition::sq64(engine::square::d3));
    b.set_sq(engine::transition::sq64(engine::square::d4));*/

    engine::board b;

    const FString s = b.str().c_str();
    LOG("%s", *s);

    /*while(!b.is_empty()) {
        LOG("popped: %d", b.pop());
        LOG("count: %d", b.count());
    }*/

	// ...
	
}


// Called every frame
void UNewActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

