#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrafficRoute.h"
#include "TrafficVehicle.generated.h"

class UStaticMeshComponent;


UCLASS()
class SAFESTREET_API ATrafficVehicle : public AActor
{
    GENERATED_BODY()

public:

    ATrafficVehicle();


protected:

    virtual void BeginPlay() override;


public:

    virtual void Tick(float DeltaTime) override;


protected:

    // =========================================================
    // COMPONENTES
    // =========================================================

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Traffic|Components"
    )
    TObjectPtr<UStaticMeshComponent> VehicleMesh;


    // =========================================================
    // RUTA
    // =========================================================

    UPROPERTY(
        EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Traffic|Route"
    )
    TObjectPtr<ATrafficRoute> CurrentRoute;


    UPROPERTY(
        EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Traffic|Route"
    )
    float StartDistance = 0.0f;


    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Traffic|Route"
    )
    float DistanceAlongSpline = 0.0f;


    // =========================================================
    // MOVIMIENTO
    // =========================================================

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Traffic|Movement"
    )
    float CurrentSpeed = 0.0f;


    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Traffic|Movement"
    )
    float TargetSpeed = 0.0f;


    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Movement"
    )
    float Acceleration = 250.0f;


    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Movement"
    )
    float Deceleration = 400.0f;


    // =========================================================
    // VELOCIDAD PROBABILISTICA
    // =========================================================

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Probability"
    )
    float MeanSpeedKmh = 35.0f;


    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Probability"
    )
    float SpeedStdDevKmh = 5.0f;


    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Probability"
    )
    float MinSpeedKmh = 25.0f;


    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Probability"
    )
    float MaxSpeedKmh = 45.0f;


    // =========================================================
    // SEGUIMIENTO
    // =========================================================

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Following"
    )
    float MinimumGap = 250.0f;


    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Following"
    )
    float TimeHeadway = 1.5f;


    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Following"
    )
    float DetectionDistance = 3000.0f;


    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Traffic|Following"
    )
    TObjectPtr<ATrafficVehicle> VehicleAhead;


private:

    ATrafficVehicle* FindVehicleAhead() const;

    float CalculateSafeDistance() const;

    float CalculateTrafficLightSpeed(
        float CurrentDesiredSpeed
    ) const;

    float GenerateNormalRandom(
        float Mean,
        float StandardDeviation
    );

    float KmhToCms(float Kmh) const;
};