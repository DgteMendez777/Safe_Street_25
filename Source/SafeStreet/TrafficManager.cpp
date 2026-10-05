#include "TrafficManager.h"

#include "TrafficVehicle.h"
#include "TrafficRoute.h"

#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"


// =============================================================
// CONSTRUCTOR
// =============================================================

ATrafficManager::ATrafficManager()
{
    PrimaryActorTick.bCanEverTick = false;
}


// =============================================================
// BEGIN PLAY
// =============================================================

void ATrafficManager::BeginPlay()
{
    Super::BeginPlay();


    if (!bAutoStart)
    {
        return;
    }


    if (VehicleClasses.Num() == 0)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "TrafficManager no tiene VehicleClasses."
            )
        );

        return;
    }


    if (SpawnRoutes.Num() == 0)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "TrafficManager no tiene SpawnRoutes."
            )
        );

        return;
    }


    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "TrafficManager iniciado | "
            "%.2f vehiculos/minuto/ruta | "
            "%d rutas | "
            "%d tipos de vehiculo"
        ),
        VehiclesPerMinute,
        SpawnRoutes.Num(),
        VehicleClasses.Num()
    );


    ScheduleNextSpawn();
}


// =============================================================
// PROGRAMAR SIGUIENTE SPAWN
// =============================================================

void ATrafficManager::ScheduleNextSpawn()
{
    const float NextInterval =
        GenerateNextSpawnInterval();


    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "Proximo intento de spawn en %.2f segundos."
        ),
        NextInterval
    );


    GetWorldTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &ATrafficManager::TrySpawnVehicle,
        NextInterval,
        false
    );
}


// =============================================================
// INTERVALO EXPONENCIAL
// =============================================================

float ATrafficManager::GenerateNextSpawnInterval() const
{
    /*
        Proceso de Poisson.

        Si lambda representa vehiculos por segundo:

              T = -ln(U) / lambda

        donde:

              U ~ Uniforme(0,1)

        VehiclesPerMinute esta expresado
        en vehiculos/minuto.
    */


    const float SafeVehiclesPerMinute =
        FMath::Max(
            VehiclesPerMinute,
            0.1f
        );


    const float LambdaPerSecond =
        SafeVehiclesPerMinute
        /
        60.0f;


    const float U =
        FMath::FRandRange(
            0.0001f,
            0.9999f
        );


    const float Interval =
        -FMath::Loge(U)
        /
        LambdaPerSecond;


    // Evitamos intervalos extremadamente pequenos
    // que puedan provocar muchos intentos seguidos.
    return FMath::Max(
        Interval,
        0.5f
    );
}


// =============================================================
// INTENTAR SPAWN
// =============================================================

void ATrafficManager::TrySpawnVehicle()
{
    // ---------------------------------------------------------
    // Seleccionar ruta
    // ---------------------------------------------------------

    ATrafficRoute* SelectedRoute =
        SelectRandomRoute();


    if (!SelectedRoute)
    {
        ScheduleNextSpawn();
        return;
    }


    // ---------------------------------------------------------
    // Comprobar espacio
    // ---------------------------------------------------------

    if (!IsRouteSpawnClear(SelectedRoute))
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "Spawn cancelado: inicio de ruta ocupado."
            )
        );


        ScheduleNextSpawn();
        return;
    }


    // ---------------------------------------------------------
    // Seleccionar tipo de vehiculo
    // ---------------------------------------------------------

    TSubclassOf<ATrafficVehicle> SelectedClass =
        SelectRandomVehicleClass();


    if (!SelectedClass)
    {
        ScheduleNextSpawn();
        return;
    }


    // ---------------------------------------------------------
    // Obtener Spline
    // ---------------------------------------------------------

    USplineComponent* Spline =
        SelectedRoute->GetSpline();


    if (!Spline)
    {
        ScheduleNextSpawn();
        return;
    }


    // ---------------------------------------------------------
    // Transform inicial
    // ---------------------------------------------------------

    const FVector SpawnLocation =
        Spline->GetLocationAtDistanceAlongSpline(
            0.0f,
            ESplineCoordinateSpace::World
        );


    const FRotator SpawnRotation =
        Spline->GetRotationAtDistanceAlongSpline(
            0.0f,
            ESplineCoordinateSpace::World
        );


    // =========================================================
    // SPAWN DIFERIDO
    // =========================================================

    /*
        Usamos SpawnActorDeferred porque necesitamos
        asignar CurrentRoute ANTES de BeginPlay.

        Esto es importante.

        Si usaramos SpawnActor directamente, BeginPlay
        podria ejecutarse antes de asignar la ruta.
    */


    FTransform SpawnTransform(
        SpawnRotation,
        SpawnLocation
    );


    ATrafficVehicle* NewVehicle =
        GetWorld()->SpawnActorDeferred<ATrafficVehicle>(
            SelectedClass,
            SpawnTransform,
            this,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );


    if (NewVehicle)
    {
        // El Manager necesita poder configurar estas
        // propiedades antes de BeginPlay.
        NewVehicle->SetSpawnRoute(
            SelectedRoute,
            0.0f
        );


        NewVehicle->FinishSpawning(
            SpawnTransform
        );


        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Vehiculo generado en ruta %s."
            ),
            *SelectedRoute->GetName()
        );
    }


    // ---------------------------------------------------------
    // Programar siguiente llegada
    // ---------------------------------------------------------

    ScheduleNextSpawn();
}


// =============================================================
// COMPROBAR INICIO DE RUTA
// =============================================================

bool ATrafficManager::IsRouteSpawnClear(
    ATrafficRoute* Route
) const
{
    if (!Route)
    {
        return false;
    }


    for (
        TActorIterator<ATrafficVehicle> It(GetWorld());
        It;
        ++It
        )
    {
        ATrafficVehicle* Vehicle =
            *It;


        if (!Vehicle)
        {
            continue;
        }


        if (
            Vehicle->GetCurrentRoute()
            !=
            Route
            )
        {
            continue;
        }


        // Si hay un vehiculo demasiado cerca
        // del comienzo de la ruta, no hacemos spawn.
        if (
            Vehicle->GetDistanceAlongSpline()
            <
            MinimumSpawnClearance
            )
        {
            return false;
        }
    }


    return true;
}


// =============================================================
// SELECCIONAR RUTA
// =============================================================

ATrafficRoute*
ATrafficManager::SelectRandomRoute() const
{
    if (SpawnRoutes.Num() == 0)
    {
        return nullptr;
    }


    const int32 RandomIndex =
        FMath::RandRange(
            0,
            SpawnRoutes.Num() - 1
        );


    return SpawnRoutes[RandomIndex];
}


// =============================================================
// SELECCIONAR VEHICULO
// =============================================================

TSubclassOf<ATrafficVehicle>
ATrafficManager::SelectRandomVehicleClass() const
{
    if (VehicleClasses.Num() == 0)
    {
        return nullptr;
    }


    const int32 RandomIndex =
        FMath::RandRange(
            0,
            VehicleClasses.Num() - 1
        );


    return VehicleClasses[RandomIndex];
}