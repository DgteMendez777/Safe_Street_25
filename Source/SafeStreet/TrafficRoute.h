#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "SafeStreetVehicleTrafficLight.h"
#include "TrafficRoute.generated.h"


// =============================================================
// CONTROL DE TRAFICO SOBRE UNA RUTA
// =============================================================
//
// Cada elemento representa un semaforo situado en algun punto
// de la Spline.
//
// Ejemplo:
//
// Semaforo A -> StopDistance = 6000
// Semaforo B -> StopDistance = 14500
// Semaforo C -> StopDistance = 23000
//
// =============================================================

USTRUCT(BlueprintType)
struct FTrafficLightControlPoint
{
    GENERATED_BODY()

    // Semaforo que controla este punto.
    UPROPERTY(
        EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Traffic"
    )
    TObjectPtr<ASafeStreetVehicleTrafficLight> TrafficLight = nullptr;


    // Posicion de la linea de parada sobre la Spline.
    // Unidad: centimetros.
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic",
        meta = (ClampMin = "0.0")
    )
    float StopDistance = 0.0f;


    // Distancia desde la que los vehiculos empiezan
    // a reaccionar al semaforo.
    // 3000 cm = 30 metros.
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic",
        meta = (ClampMin = "0.0")
    )
    float DetectionDistance = 3000.0f;


    // Margen antes de la linea de parada.
    // 100 cm = 1 metro.
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic",
        meta = (ClampMin = "0.0")
    )
    float StopLineMargin = 100.0f;
};


UCLASS()
class SAFESTREET_API ATrafficRoute : public AActor
{
    GENERATED_BODY()

public:

    ATrafficRoute();


    // =========================================================
    // SPLINE
    // =========================================================

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Traffic|Route"
    )
    TObjectPtr<USplineComponent> Spline;


    // =========================================================
    // SEMAFOROS DE ESTA RUTA
    // =========================================================

    UPROPERTY(
        EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Traffic|Traffic Lights"
    )
    TArray<FTrafficLightControlPoint> TrafficLightControls;


    // =========================================================
    // GETTERS
    // =========================================================

    UFUNCTION(
        BlueprintPure,
        Category = "Traffic|Route"
    )
    USplineComponent* GetSpline() const
    {
        return Spline;
    }


    // Busca el siguiente semaforo que el vehiculo
    // encontrara sobre esta ruta.
    const FTrafficLightControlPoint*
        GetNextTrafficLight(
            float CurrentDistance
        ) const;
};