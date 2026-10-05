#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrafficManager.generated.h"

class ATrafficVehicle;
class ATrafficRoute;


UCLASS()
class SAFESTREET_API ATrafficManager : public AActor
{
    GENERATED_BODY()

public:

    ATrafficManager();


protected:

    virtual void BeginPlay() override;


    // =========================================================
    // VEHICULOS DISPONIBLES
    // =========================================================

    // Blueprints/clases de vehiculos que el Manager
    // puede generar.
    //
    // Ejemplo:
    // BP_Car_Red
    // BP_Jeep
    // BP_Sedan
    // BP_Van
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Vehicles"
    )
    TArray<TSubclassOf<ATrafficVehicle>> VehicleClasses;


    // =========================================================
    // RUTAS DE ENTRADA
    // =========================================================

    // Por ahora el Manager puede generar vehiculos
    // en cualquiera de estas rutas.
    UPROPERTY(
        EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Traffic|Routes"
    )
    TArray<TObjectPtr<ATrafficRoute>> SpawnRoutes;


    // =========================================================
    // FLUJO DE TRAFICO
    // =========================================================

    // Tasa media de llegada:
    // vehiculos por minuto POR RUTA.
    //
    // Ejemplo:
    // 12 = aproximadamente 12 vehiculos/minuto/ruta.
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Spawn",
        meta = (ClampMin = "0.1")
    )
    float VehiclesPerMinute = 12.0f;


    // Si esta activo, comienza a generar trafico
    // automaticamente al iniciar el nivel.
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Spawn"
    )
    bool bAutoStart = true;


    // Distancia minima libre desde el inicio de la ruta
    // para permitir un nuevo spawn.
    //
    // 800 cm = 8 metros.
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Traffic|Spawn",
        meta = (ClampMin = "0.0")
    )
    float MinimumSpawnClearance = 800.0f;


private:

    // Intenta generar un vehiculo.
    void TrySpawnVehicle();


    // Genera el siguiente intervalo usando
    // una distribucion exponencial.
    float GenerateNextSpawnInterval() const;


    // Programa el siguiente intento de spawn.
    void ScheduleNextSpawn();


    // Comprueba que el inicio de una ruta
    // tenga espacio suficiente.
    bool IsRouteSpawnClear(
        ATrafficRoute* Route
    ) const;


    // Selecciona aleatoriamente una ruta.
    ATrafficRoute* SelectRandomRoute() const;


    // Selecciona aleatoriamente una clase de vehiculo.
    TSubclassOf<ATrafficVehicle>
        SelectRandomVehicleClass() const;


    FTimerHandle SpawnTimerHandle;
};