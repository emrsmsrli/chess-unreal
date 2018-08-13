// Copyright 2018 Emre Simsirli

#pragma once

#include "GameFramework/Pawn.h"
#include "ChessPlayer.generated.h"

UCLASS()
class CHESS_API AChessPlayer : public APawn {
    GENERATED_BODY()

public:
    AChessPlayer();

protected:
    void BeginPlay() override;

public:
    void Tick(float DeltaTime) override;
    void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    void ZoomCamera(float amount);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;
    class UCameraComponent* Camera;
};
