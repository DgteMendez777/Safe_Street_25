#include "TrafficRoute.h"


// =============================================================
// CONSTRUCTOR
// =============================================================

ATrafficRoute::ATrafficRoute()
{
    PrimaryActorTick.bCanEverTick = false;

    Spline =
        CreateDefaultSubobject<USplineComponent>(
            TEXT("TrafficSpline")
        );

    RootComponent = Spline;
}


// =============================================================
// BUSCAR PROXIMO SEMAFORO
// =============================================================

const FTrafficLightControlPoint*
ATrafficRoute::GetNextTrafficLight(
    float CurrentDistance
) const
{
    const FTrafficLightControlPoint* ClosestControl = nullptr;

    float ClosestDistance =
        TNumericLimits<float>::Max();


    // Recorremos todos los semaforos asociados
    // a esta ruta.
    for (
        const FTrafficLightControlPoint& Control
        :
        TrafficLightControls
        )
    {
        // Ignorar elementos sin semaforo.
        if (!Control.TrafficLight)
        {
            continue;
        }


        // Distancia desde el vehiculo
        // hasta este semaforo.
        const float DistanceAhead =
            Control.StopDistance
            -
            CurrentDistance;


        // Si es negativo, ya pasamos ese semaforo.
        if (DistanceAhead < 0.0f)
        {
            continue;
        }


        // Nos quedamos con el semaforo
        // mas cercano que este delante.
        if (DistanceAhead < ClosestDistance)
        {
            ClosestDistance =
                DistanceAhead;

            ClosestControl =
                &Control;
        }
    }


    return ClosestControl;
}