// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

UCLASS()
class CHESS_API APlayerPawn : public APawn {
    GENERATED_BODY()

public:
    APlayerPawn();

protected:
    void BeginPlay() override;

public:
    void Tick(float DeltaTime) override;
    void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void ZoomCamera(float amount) const;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;
    class UCameraComponent* Camera;
};
