// Copyright 2018 Emre Simsirli

#include "Player/ChessPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Debug.h"
#include "ChessPlayer.h"

AChessPlayerController::AChessPlayerController() {
    IsCamControlActive = false;
    bShowMouseCursor = true;
    bEnableMouseOverEvents = true;

    RotationMultiplier = 30;
    ZoomMultiplier = 15;
    // todo PlayerCameraManagerClass = nullptr;
}

void AChessPlayerController::Tick(float DeltaSeconds) {
    if(GEngine) {
#ifdef DEBUG
        GEngine->AddOnScreenDebugMessage(4, 0, FColor::Blue, "control rotation " + GetControlRotation().ToString());
#endif

        const auto vp_size = GEngine->GameViewport->Viewport->GetSizeXY();
        if(IsCamControlActive) {
            GEngine->GameViewport->Viewport->SetMouse(vp_size.X / 2, vp_size.Y / 2);
        }
    }

    auto rot = GetControlRotation();
    rot.Pitch = FMath::Clamp(rot.Pitch, 280.0f, 350.0f);
    SetControlRotation(rot);
}

void AChessPlayerController::BeginPlay() {
    APlayerController::BeginPlay();
    SetControlRotation(FRotator(330.0f, 0, 0));
}

void AChessPlayerController::SetupInputComponent() {
    APlayerController::SetupInputComponent();
    check(InputComponent);

    InputComponent->BindAction("ToggleCameraInput", EInputEvent::IE_Pressed, this,
                               &AChessPlayerController::OnToggleCameraControl);
    InputComponent->BindAction("ToggleCameraInput", EInputEvent::IE_Released, this,
                               &AChessPlayerController::OnToggleCameraControl);
    InputComponent->BindAxis("CameraPitch", this, &AChessPlayerController::OnCameraPitch);
    InputComponent->BindAxis("CameraYaw", this, &AChessPlayerController::OnCameraYaw);
    InputComponent->BindAxis("CameraZoom", this, &AChessPlayerController::OnCameraZoom);
}

void AChessPlayerController::OnToggleCameraControl() {
    if(GEngine && !IsCamControlActive) {
        previous_mouse_x_ = GEngine->GameViewport->Viewport->GetMouseX();
        previous_mouse_y_ = GEngine->GameViewport->Viewport->GetMouseY();
    } else {
        GEngine->GameViewport->Viewport->SetMouse(previous_mouse_x_, previous_mouse_y_);
    }

    IsCamControlActive = !IsCamControlActive;
    bShowMouseCursor = !bShowMouseCursor;
    bEnableMouseOverEvents = !bEnableMouseOverEvents;
}

void AChessPlayerController::OnCameraPitch(const float value) {
    if(!IsCamControlActive)
        return;

#ifdef DEBUG
    if(GEngine)
        GEngine->AddOnScreenDebugMessage(1, 0, FColor::Black, "pitch " + FString::SanitizeFloat(value));
#endif

    AddPitchInput(value * GetWorld()->GetDeltaSeconds() * RotationMultiplier);
}

void AChessPlayerController::OnCameraYaw(const float value) {
    if(!IsCamControlActive)
        return;

#ifdef DEBUG
    if(GEngine)
        GEngine->AddOnScreenDebugMessage(2, 0, FColor::Black, "yaw " + FString::SanitizeFloat(value));
#endif

    AddYawInput(value * GetWorld()->GetDeltaSeconds() * RotationMultiplier);
}

void AChessPlayerController::OnCameraZoom(const float value) {
#ifdef DEBUG
    if(GEngine)
        GEngine->AddOnScreenDebugMessage(3, 0, FColor::Black, "zoom " + FString::SanitizeFloat(value));
#endif

    auto* pawn = Cast<AChessPlayer>(GetPawn());
    pawn->ZoomCamera(value * ZoomMultiplier);
}
