// Copyright 2018 Emre Simsirli

#include "Player/PlayerPawnController.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Debug.h"
#include "PlayerPawn.h"

APlayerPawnController::APlayerPawnController() {
    IsCamControlActive = false;
    bShowMouseCursor = true;
    bEnableMouseOverEvents = true;

    RotationMultiplier = 30;
    ZoomMultiplier = 15;
    // todo PlayerCameraManagerClass = nullptr;
}

void APlayerPawnController::Tick(float DeltaSeconds) {
    if(GEngine)
        GEngine->AddOnScreenDebugMessage(4, 0, FColor::Blue, "control rotation " + GetControlRotation().ToString());
    const auto vp_size = GEngine->GameViewport->Viewport->GetSizeXY();
    if(IsCamControlActive) {
        GEngine->GameViewport->Viewport->SetMouse(vp_size.X / 2, vp_size.Y / 2);

        auto rot = GetControlRotation();
        rot.Pitch = FMath::Clamp(rot.Pitch, 280.0f, 350.0f);
        SetControlRotation(rot);
    }
}

void APlayerPawnController::BeginPlay() {
    APlayerController::BeginPlay();
    SetControlRotation(FRotator(330.0f, 0, 0));
}

void APlayerPawnController::SetupInputComponent() {
    APlayerController::SetupInputComponent();
    check(InputComponent);

    InputComponent->BindAction("ToggleCameraInput", EInputEvent::IE_Pressed, this,
                               &APlayerPawnController::OnToggleCameraControl);
    InputComponent->BindAction("ToggleCameraInput", EInputEvent::IE_Released, this,
                               &APlayerPawnController::OnToggleCameraControl);
    InputComponent->BindAxis("CameraPitch", this, &APlayerPawnController::OnCameraPitch);
    InputComponent->BindAxis("CameraYaw", this, &APlayerPawnController::OnCameraYaw);
    InputComponent->BindAxis("CameraZoom", this, &APlayerPawnController::OnCameraZoom);
}

void APlayerPawnController::OnToggleCameraControl() {
    if(!IsCamControlActive) {
        previous_mouse_x_ = GEngine->GameViewport->Viewport->GetMouseX();
        previous_mouse_y_ = GEngine->GameViewport->Viewport->GetMouseY();
    } else {
        GEngine->GameViewport->Viewport->SetMouse(previous_mouse_x_, previous_mouse_y_);
    }

    IsCamControlActive = !IsCamControlActive;
    bShowMouseCursor = !bShowMouseCursor;
    bEnableMouseOverEvents = !bEnableMouseOverEvents;
}

void APlayerPawnController::OnCameraPitch(const float value) {
    if(!IsCamControlActive)
        return;

#ifdef DEBUG
    GEngine->AddOnScreenDebugMessage(1, 0, FColor::Black, "pitch " + FString::SanitizeFloat(value));
#endif

    AddPitchInput(value * GetWorld()->GetDeltaSeconds() * RotationMultiplier);
}

void APlayerPawnController::OnCameraYaw(const float value) {
    if(!IsCamControlActive)
        return;

#ifdef DEBUG
    GEngine->AddOnScreenDebugMessage(2, 0, FColor::Black, "yaw " + FString::SanitizeFloat(value));
#endif

    AddYawInput(value * GetWorld()->GetDeltaSeconds() * RotationMultiplier);
}

void APlayerPawnController::OnCameraZoom(const float value) {
#ifdef DEBUG
    GEngine->AddOnScreenDebugMessage(3, 0, FColor::Black, "zoom " + FString::SanitizeFloat(value));
#endif

    auto* pawn = Cast<APlayerPawn>(GetPawn());
    pawn->ZoomCamera(value * ZoomMultiplier);
}
