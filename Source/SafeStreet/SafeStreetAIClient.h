#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "TimerManager.h"
#include "SafeStreetAIClient.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;


// ============================================================
// ESTRUCTURA DE UNA DETECCION DE LA IA
// ============================================================

USTRUCT(BlueprintType)
struct FSafeStreetDetection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ClassId = -1;

	UPROPERTY(BlueprintReadOnly)
	FString ClassName;

	UPROPERTY(BlueprintReadOnly)
	float Confidence = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float X1 = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float Y1 = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float X2 = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float Y2 = 0.0f;
};


// ============================================================
// CLIENTE DE IA DE SAFESTREET
// ============================================================

UCLASS()
class SAFESTREET_API ASafeStreetAIClient : public AActor
{
	GENERATED_BODY()

public:

	ASafeStreetAIClient();

	virtual void Tick(float DeltaTime) override;


protected:

	virtual void BeginPlay() override;


private:

	// ========================================================
	// CAMARA DE IA
	// ========================================================

	UPROPERTY(
		VisibleAnywhere,
		Category = "SafeStreet AI|Camera"
	)
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;


	UPROPERTY(
		EditAnywhere,
		Category = "SafeStreet AI|Camera"
	)
	TObjectPtr<UTextureRenderTarget2D> CameraRenderTarget;


	void ConfigureAICamera();


	// ========================================================
	// SERVIDOR IA
	// ========================================================

	void CheckServerHealth();

	void OnHealthResponse(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bWasSuccessful
	);


	// ========================================================
	// INFERENCIA PERIODICA
	// ========================================================

	void TrySendCameraFrame();

	bool CaptureCameraFrameToPNG(
		TArray64<uint8>& OutPNGData
	);

	void SendFrameToAI(
		const TArray64<uint8>& PNGData
	);


	void OnDetectionResponse(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bWasSuccessful
	);


	// ========================================================
	// JSON
	// ========================================================

	void ParseDetectionResponse(
		const FString& JsonResponse
	);


	// ========================================================
	// ESTADO
	// ========================================================

	FTimerHandle AIInferenceTimerHandle;

	bool bServerConnected = false;

	bool bAIRequestInProgress = false;


	// Intervalo inicial entre intentos de inferencia.
	// Empezamos con 2 segundos para mantener el sistema ligero.

	UPROPERTY(
		EditAnywhere,
		Category = "SafeStreet AI|Detection"
	)
	float InferenceInterval = 2.0f;


	// ========================================================
	// RESULTADOS
	// ========================================================

	UPROPERTY()
	TArray<FSafeStreetDetection> LastDetections;
};