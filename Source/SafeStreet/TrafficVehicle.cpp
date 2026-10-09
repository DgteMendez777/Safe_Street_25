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

    // The vehicle teleports each tick (no sweep), so overlap is what reliably detects
    // the player regardless of the imported mesh's own collision setup.
    VehicleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    VehicleMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    VehicleMesh->SetGenerateOverlapEvents(true);
}


// =============================================================
// CONFIGURACION DESDE TRAFFIC MANAGER
// =============================================================

void ATrafficVehicle::SetSpawnRoute(
    ATrafficRoute* NewRoute,
    float NewStartDistance
)
{
    CurrentRoute = NewRoute;

    StartDistance = NewStartDistance;

    DistanceAlongSpline =
        NewStartDistance;
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
            "%s -> Ruta: %s | "
            "Velocidad objetivo: %.2f km/h"
        ),
        *GetName(),
        *CurrentRoute->GetName(),
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
    // VELOCIDAD DESEADA BASE
    // =========================================================

    float DesiredSpeed =
        TargetSpeed;


    // =========================================================
    // VEHICULO DELANTERO
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
    // SEMAFOROS
    // =========================================================

    DesiredSpeed =
        CalculateTrafficLightSpeed(
            DesiredSpeed
        );


    // =========================================================
    // ACELERACION / FRENADO
    // =========================================================

    if (CurrentSpeed < DesiredSpeed)
    {
        CurrentSpeed =
            FMath::FInterpConstantTo(
                CurrentSpeed,
                DesiredSpeed,
                DeltaTime,
                Acceleration
            );
    }
    else if (CurrentSpeed > DesiredSpeed)
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
    // MOVIMIENTO
    // =========================================================

    DistanceAlongSpline +=
        CurrentSpeed
        *
        DeltaTime;


    const float SplineLength =
        Spline->GetSplineLength();


    if (SplineLength <= 0.0f)
    {
        return;
    }


    // TEMPORAL.
    // Luego el Manager eliminara el vehiculo
    // al finalizar su recorrido.
    if (
        DistanceAlongSpline
        >=
        SplineLength
    )
    {
        DistanceAlongSpline = 0.0f;
    }


    // =========================================================
    // POSICION / ROTACION
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


    const FTrafficLightControlPoint* Control =
        CurrentRoute->GetNextTrafficLight(
            DistanceAlongSpline
        );


    if (!Control)
    {
        return CurrentDesiredSpeed;
    }


    if (!Control->TrafficLight)
    {
        return CurrentDesiredSpeed;
    }


    const float DistanceToStopLine =
        Control->StopDistance
        -
        DistanceAlongSpline;


    if (
        DistanceToStopLine
        >
        Control->DetectionDistance
    )
    {
        return CurrentDesiredSpeed;
    }


    const EVehicleTrafficLightState LightState =
        Control->TrafficLight->GetLightState();


    // Verde -> no hay restriccion.
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


    if (
        EffectiveDistance
        <=
        10.0f
    )
    {
        return 0.0f;
    }


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