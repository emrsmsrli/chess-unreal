// Copyright 2018 Emre Simsirli

#pragma once

#include "GameFramework/PlayerController.h"
#include "ChessPlayerController.generated.h"

UCLASS()
class CHESS_API AChessPlayerController : public APlayerController {
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Camera", Config = settings, meta = (ClampMin = 0))
    float RotationMultiplier;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Camera", Config = settings, meta = (ClampMin = 0))
    float ZoomMultiplier;

private:
    bool IsCamControlActive;

    int32 previous_mouse_x_ = -1;
    int32 previous_mouse_y_ = -1;

public:
    AChessPlayerController();
    void Tick(float DeltaSeconds) override;

protected:
    void BeginPlay() override;
    void SetupInputComponent() override;

private:
    void OnToggleCameraControl();
    void OnCameraPitch(float value);
    void OnCameraYaw(float value);
    void OnCameraZoom(float value);
};
