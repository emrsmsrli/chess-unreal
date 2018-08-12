// Copyright 2018 Emre Simsirli

#include "Player/ChessPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"

AChessPlayer::AChessPlayer() {
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;

    RootComponent = CreateDefaultSubobject<class USceneComponent>(TEXT("root"));
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("camera_boom"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("camera"));

    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetRelativeLocationAndRotation(FVector(0, 0, 10.0f), FRotator(0, 0, 0));
    CameraBoom->TargetArmLength = 120.0f;
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bUsePawnControlRotation = true;
    
    Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
}

void AChessPlayer::BeginPlay() {
    APawn::BeginPlay();
}

void AChessPlayer::Tick(const float DeltaTime) {
    APawn::Tick(DeltaTime);
}
    
void AChessPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    APawn::SetupPlayerInputComponent(PlayerInputComponent);
}

void AChessPlayer::ZoomCamera(const float amount) const {
    CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength + amount, 75.0f, 200.0f);
}
