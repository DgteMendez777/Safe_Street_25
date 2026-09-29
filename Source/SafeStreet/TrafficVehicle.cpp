#include "TrafficVehicle.h"

#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"


// =============================================================
// CONSTRUCTOR
// =============================================================

ATrafficVehicle::ATrafficVehicle()
{
    PrimaryActorTick.bCanEverTick = true;

    VehicleMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("VehicleMesh")
        );

    RootComponent = VehicleMesh;

    VehicleMesh->SetMobility(
        EComponentMobility::Movable
    );
}


// =============================================================
// BEGIN PLAY
// =============================================================

void ATrafficVehicle::BeginPlay()
{
    Super::BeginPlay();


    if (!CurrentRoute)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "%s no tiene CurrentRoute asignado."
            ),
            *GetName()
        );

        return;
    }


    // Posicion inicial.
    DistanceAlongSpline =
        StartDistance;


    // =========================================================
    // VELOCIDAD PROBABILISTICA
    // =========================================================

    float RandomSpeedKmh =
        GenerateNormalRandom(
            MeanSpeedKmh,
            SpeedStdDevKmh
        );


    RandomSpeedKmh =
        FMath::Clamp(
            RandomSpeedKmh,
            MinSpeedKmh,
            MaxSpeedKmh
        );


    TargetSpeed =
        KmhToCms(
            RandomSpeedKmh
        );


    CurrentSpeed = 0.0f;


    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "%s -> Inicio: %.0f cm | "
            "Velocidad objetivo: %.2f km/h"
        ),
        *GetName(),
        StartDistance,
        RandomSpeedKmh
    );
}


// =============================================================
// TICK
// =============================================================

void ATrafficVehicle::Tick(
    float DeltaTime
)
{
    Super::Tick(DeltaTime);


    if (!CurrentRoute)
    {
        return;
    }


    USplineComponent* Spline =
        CurrentRoute->GetSpline();


    if (!Spline)
    {
        return;
    }


    // =========================================================
    // 1. VELOCIDAD DESEADA
    // =========================================================

    float DesiredSpeed =
        TargetSpeed;


    // =========================================================
    // 2. VEHICULO DELANTERO
    // =========================================================

    VehicleAhead =
        FindVehicleAhead();


    if (VehicleAhead)
    {
        const float DistanceToVehicle =
            VehicleAhead->DistanceAlongSpline
            -
            DistanceAlongSpline;


        const float SafeDistance =
            CalculateSafeDistance();


        if (
            DistanceToVehicle
            <=
            SafeDistance
            )
        {
            DesiredSpeed =
                FMath::Min(
                    DesiredSpeed,
                    VehicleAhead->CurrentSpeed
                );
        }


        if (
            DistanceToVehicle
            <=
            MinimumGap
            )
        {
            DesiredSpeed = 0.0f;
        }
    }


    // =========================================================
    // 3. SEMAFOROS
    // =========================================================

    DesiredSpeed =
        CalculateTrafficLightSpeed(
            DesiredSpeed
        );


    // =========================================================
    // 4. ACELERACION / FRENADO
    // =========================================================

    if (
        CurrentSpeed
        <
        DesiredSpeed
        )
    {
        CurrentSpeed =
            FMath::FInterpConstantTo(
                CurrentSpeed,
                DesiredSpeed,
                DeltaTime,
                Acceleration
            );
    }
    else if (
        CurrentSpeed
        >
        DesiredSpeed
        )
    {
        CurrentSpeed =
            FMath::FInterpConstantTo(
                CurrentSpeed,
                DesiredSpeed,
                DeltaTime,
                Deceleration
            );
    }


    CurrentSpeed =
        FMath::Max(
            CurrentSpeed,
            0.0f
        );


    // =========================================================
    // 5. AVANZAR
    // =========================================================

    DistanceAlongSpline +=
        CurrentSpeed
        *
        DeltaTime;


    // =========================================================
    // 6. LONGITUD DE RUTA
    // =========================================================

    const float SplineLength =
        Spline->GetSplineLength();


    if (SplineLength <= 0.0f)
    {
        return;
    }


    // TEMPORAL PARA PRUEBAS.
    if (
        DistanceAlongSpline
        >=
        SplineLength
        )
    {
        DistanceAlongSpline = 0.0f;
    }


    // =========================================================
    // 7. TRANSFORM
    // =========================================================

    const FVector NewLocation =
        Spline->GetLocationAtDistanceAlongSpline(
            DistanceAlongSpline,
            ESplineCoordinateSpace::World
        );


    const FRotator NewRotation =
        Spline->GetRotationAtDistanceAlongSpline(
            DistanceAlongSpline,
            ESplineCoordinateSpace::World
        );


    SetActorLocationAndRotation(
        NewLocation,
        NewRotation
    );
}


// =============================================================
// SEMAFOROS
// =============================================================

float ATrafficVehicle::CalculateTrafficLightSpeed(
    float CurrentDesiredSpeed
) const
{
    if (!CurrentRoute)
    {
        return CurrentDesiredSpeed;
    }


    // =========================================================
    // BUSCAR PROXIMO SEMAFORO
    // =========================================================

    const FTrafficLightControlPoint* Control =
        CurrentRoute->GetNextTrafficLight(
            DistanceAlongSpline
        );


    // Esta ruta no tiene mas semaforos delante.
    if (!Control)
    {
        return CurrentDesiredSpeed;
    }


    if (!Control->TrafficLight)
    {
        return CurrentDesiredSpeed;
    }


    // =========================================================
    // DISTANCIA HASTA LA LINEA
    // =========================================================

    const float DistanceToStopLine =
        Control->StopDistance
        -
        DistanceAlongSpline;


    // Todavia estamos demasiado lejos.
    if (
        DistanceToStopLine
        >
        Control->DetectionDistance
        )
    {
        return CurrentDesiredSpeed;
    }


    // =========================================================
    // ESTADO
    // =========================================================

    const EVehicleTrafficLightState LightState =
        Control->TrafficLight->GetLightState();


    // Verde: continuar normalmente.
    if (
        LightState
        ==
        EVehicleTrafficLightState::Green
        )
    {
        return CurrentDesiredSpeed;
    }


    // =========================================================
    // ROJO / AMARILLO
    // =========================================================

    const float EffectiveDistance =
        FMath::Max(
            DistanceToStopLine
            -
            Control->StopLineMargin,
            0.0f
        );


    // Estamos practicamente en la linea.
    if (
        EffectiveDistance
        <=
        10.0f
        )
    {
        return 0.0f;
    }


    /*
        Cinematica:

        v² = 2ad

        v = sqrt(2ad)
    */

    const float MaxStoppingSpeed =
        FMath::Sqrt(
            2.0f
            *
            Deceleration
            *
            EffectiveDistance
        );


    return
        FMath::Min(
            CurrentDesiredSpeed,
            MaxStoppingSpeed
        );
}


// =============================================================
// VEHICULO DELANTERO
// =============================================================

ATrafficVehicle*
ATrafficVehicle::FindVehicleAhead() const
{
    ATrafficVehicle* ClosestVehicle =
        nullptr;


    float ClosestDistance =
        DetectionDistance;


    for (
        TActorIterator<ATrafficVehicle>
        It(GetWorld());
        It;
        ++It
        )
    {
        ATrafficVehicle* OtherVehicle =
            *It;


        if (
            OtherVehicle
            ==
            this
            )
        {
            continue;
        }


        if (
            OtherVehicle->CurrentRoute
            !=
            CurrentRoute
            )
        {
            continue;
        }


        const float DistanceDifference =
            OtherVehicle->DistanceAlongSpline
            -
            DistanceAlongSpline;


        if (
            DistanceDifference > 0.0f
            &&
            DistanceDifference < ClosestDistance
            )
        {
            ClosestDistance =
                DistanceDifference;


            ClosestVehicle =
                OtherVehicle;
        }
    }


    return ClosestVehicle;
}


// =============================================================
// DISTANCIA SEGURA
// =============================================================

float
ATrafficVehicle::CalculateSafeDistance() const
{
    return
        MinimumGap
        +
        CurrentSpeed
        *
        TimeHeadway;
}


// =============================================================
// CONVERSION
// =============================================================

float ATrafficVehicle::KmhToCms(
    float Kmh
) const
{
    return
        Kmh
        *
        27.777778f;
}


// =============================================================
// DISTRIBUCION NORMAL
// =============================================================

float ATrafficVehicle::GenerateNormalRandom(
    float Mean,
    float StandardDeviation
)
{
    const float U1 =
        FMath::FRandRange(
            0.0001f,
            1.0f
        );


    const float U2 =
        FMath::FRandRange(
            0.0001f,
            1.0f
        );


    const float Z =
        FMath::Sqrt(
            -2.0f
            *
            FMath::Loge(U1)
        )
        *
        FMath::Cos(
            2.0f
            *
            PI
            *
            U2
        );


    return
        Mean
        +
        StandardDeviation
        *
        Z;
}