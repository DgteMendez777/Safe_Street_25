#include "SafeStreetAIClient.h"

// HTTP
#include "HttpModule.h"

// ENGINE
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"

// CAMARA
#include "Components/SceneCaptureComponent2D.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// JSON
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

// IMAGEN
#include "ImageUtils.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

ASafeStreetAIClient::ASafeStreetAIClient()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneCapture =
		CreateDefaultSubobject<USceneCaptureComponent2D>(
			TEXT("AI_SceneCapture")
		);

	RootComponent = SceneCapture;

	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = true;
}


// ============================================================
// BEGIN PLAY
// ============================================================

void ASafeStreetAIClient::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("SafeStreet AI Client iniciado.")
	);

	ConfigureAICamera();

	CheckServerHealth();
}


// ============================================================
// TICK
// ============================================================

void ASafeStreetAIClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!SceneCapture)
	{
		return;
	}

	APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(
			GetWorld(),
			0
		);

	if (!PlayerController)
	{
		return;
	}

	APlayerCameraManager* CameraManager =
		PlayerController->PlayerCameraManager;

	if (!CameraManager)
	{
		return;
	}

	// --------------------------------------------------------
	// La camara de IA sigue a la camara real del jugador.
	// --------------------------------------------------------

	SceneCapture->SetWorldLocation(
		CameraManager->GetCameraLocation()
	);

	SceneCapture->SetWorldRotation(
		CameraManager->GetCameraRotation()
	);

	SceneCapture->FOVAngle =
		CameraManager->GetFOVAngle();
}


// ============================================================
// CONFIGURAR CAMARA IA
// ============================================================

void ASafeStreetAIClient::ConfigureAICamera()
{
	if (!SceneCapture)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("SceneCapture no existe.")
		);

		return;
	}


	if (!CameraRenderTarget)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"RT_AI_Camera no ha sido asignado "
				"al SafeStreetAIClient."
			)
		);

		return;
	}


	// --------------------------------------------------------
	// Asignar Render Target.
	// --------------------------------------------------------

	SceneCapture->TextureTarget =
		CameraRenderTarget;


	// --------------------------------------------------------
	// Obtener camara del jugador.
	// --------------------------------------------------------

	APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(
			GetWorld(),
			0
		);

	if (!PlayerController)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("No se encontro PlayerController.")
		);

		return;
	}


	APlayerCameraManager* CameraManager =
		PlayerController->PlayerCameraManager;

	if (!CameraManager)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("No se encontro PlayerCameraManager.")
		);

		return;
	}


	// --------------------------------------------------------
	// Posicion inicial.
	// --------------------------------------------------------

	const FVector CameraLocation =
		CameraManager->GetCameraLocation();

	const FRotator CameraRotation =
		CameraManager->GetCameraRotation();


	SceneCapture->SetWorldLocation(
		CameraLocation
	);

	SceneCapture->SetWorldRotation(
		CameraRotation
	);

	SceneCapture->FOVAngle =
		CameraManager->GetFOVAngle();


	// Primera captura.
	SceneCapture->CaptureScene();


	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Camara IA configurada: %dx%d"
		),
		CameraRenderTarget->SizeX,
		CameraRenderTarget->SizeY
	);
}


// ============================================================
// COMPROBAR SERVIDOR IA
// ============================================================

void ASafeStreetAIClient::CheckServerHealth()
{
	const FString ServerURL =
		TEXT("http://127.0.0.1:8000/health");


	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Conectando con SafeStreet AI Server..."
		)
	);


	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();


	Request->SetURL(
		ServerURL
	);

	Request->SetVerb(
		TEXT("GET")
	);

	Request->SetHeader(
		TEXT("Accept"),
		TEXT("application/json")
	);


	Request->OnProcessRequestComplete().BindUObject(
		this,
		&ASafeStreetAIClient::OnHealthResponse
	);


	if (!Request->ProcessRequest())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"No se pudo iniciar "
				"la peticion /health."
			)
		);
	}
}


// ============================================================
// RESPUESTA HEALTH
// ============================================================

void ASafeStreetAIClient::OnHealthResponse(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bWasSuccessful
)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		bServerConnected = false;

		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"No se pudo conectar con "
				"SafeStreet AI Server."
			)
		);

		return;
	}


	const int32 ResponseCode =
		Response->GetResponseCode();


	if (ResponseCode != 200)
	{
		bServerConnected = false;

		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"SafeStreet AI Server respondio HTTP %d"
			),
			ResponseCode
		);

		return;
	}


	bServerConnected = true;


	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"CONEXION CON LA IA CORRECTA."
		)
	);


	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Green,
			TEXT(
				"SafeStreet AI: CONECTADO"
			)
		);
	}


	// --------------------------------------------------------
	// Iniciar inferencia periodica.
	//
	// La primera inferencia se realizara despues del intervalo
	// configurado. Por defecto: 2 segundos.
	// --------------------------------------------------------

	GetWorldTimerManager().SetTimer(
		AIInferenceTimerHandle,
		this,
		&ASafeStreetAIClient::TrySendCameraFrame,
		InferenceInterval,
		true,
		InferenceInterval
	);


	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Inferencia periodica iniciada cada %.2f segundos."
		),
		InferenceInterval
	);
}


// ============================================================
// INTENTAR ENVIAR FRAME ACTUAL
// ============================================================

void ASafeStreetAIClient::TrySendCameraFrame()
{
	if (!bServerConnected)
	{
		return;
	}


	// --------------------------------------------------------
	// Si Python todavia esta procesando la solicitud anterior,
	// no enviamos otra.
	// --------------------------------------------------------

	if (bAIRequestInProgress)
	{
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT(
				"IA ocupada. Se omite esta inferencia."
			)
		);

		return;
	}


	if (!CameraRenderTarget)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"No existe CameraRenderTarget."
			)
		);

		return;
	}


	TArray64<uint8> PNGData;


	if (!CaptureCameraFrameToPNG(PNGData))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"No se pudo preparar el frame "
				"de la camara para la IA."
			)
		);

		return;
	}


	SendFrameToAI(
		PNGData
	);
}


// ============================================================
// CAPTURAR RENDER TARGET DIRECTAMENTE EN MEMORIA
// ============================================================

bool ASafeStreetAIClient::CaptureCameraFrameToPNG(
	TArray64<uint8>& OutPNGData
)
{
	if (!CameraRenderTarget)
	{
		return false;
	}


	FTextureRenderTargetResource* RenderTargetResource =
		CameraRenderTarget->GameThread_GetRenderTargetResource();


	if (!RenderTargetResource)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"No se pudo obtener "
				"RenderTargetResource."
			)
		);

		return false;
	}


	const int32 Width =
		CameraRenderTarget->SizeX;

	const int32 Height =
		CameraRenderTarget->SizeY;


	// --------------------------------------------------------
	// RT_AI_Camera utiliza RGBA16f.
	//
	// Por eso NO usamos ReadPixels() directamente.
	// Leemos los valores HDR reales como Float16.
	// --------------------------------------------------------

	TArray<FFloat16Color> FloatPixels;


	FReadSurfaceDataFlags ReadFlags(
		RCM_UNorm
	);


	const bool bReadSuccess =
		RenderTargetResource->ReadFloat16Pixels(
			FloatPixels,
			ReadFlags
		);


	if (!bReadSuccess)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"ReadFloat16Pixels fallo."
			)
		);

		return false;
	}


	if (FloatPixels.Num() != Width * Height)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Cantidad incorrecta de pixeles HDR. "
				"Esperados: %d | Recibidos: %d"
			),
			Width * Height,
			FloatPixels.Num()
		);

		return false;
	}


	// --------------------------------------------------------
	// Convertir HDR -> RGB8.
	//
	// ToFColor(true) aplica conversion a sRGB/gamma para
	// obtener una imagen normal adecuada para YOLO.
	// --------------------------------------------------------

	TArray<FColor> ColorPixels;

	ColorPixels.SetNumUninitialized(
		FloatPixels.Num()
	);


	for (
		int32 Index = 0;
		Index < FloatPixels.Num();
		++Index
		)
	{
		const FLinearColor LinearColor(
			FloatPixels[Index]
		);

		FColor FinalColor =
			LinearColor.ToFColor(true);

		// YOLO no necesita transparencia.
		FinalColor.A = 255;

		ColorPixels[Index] =
			FinalColor;
	}


	// --------------------------------------------------------
	// Comprimir como PNG directamente EN MEMORIA.
	// No se crea ningun archivo en disco.
	// --------------------------------------------------------

	OutPNGData.Reset();


	FImageUtils::PNGCompressImageArray(
		Width,
		Height,
		ColorPixels,
		OutPNGData
	);


	if (OutPNGData.Num() <= 0)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"No se pudo comprimir el frame como PNG."
			)
		);

		return false;
	}


	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Frame IA preparado: %dx%d | PNG: %lld bytes"
		),
		Width,
		Height,
		static_cast<long long>(
			OutPNGData.Num()
			)
	);


	return true;
}


// ============================================================
// ENVIAR FRAME ACTUAL A PYTHON
// ============================================================

void ASafeStreetAIClient::SendFrameToAI(
	const TArray64<uint8>& PNGData
)
{
	if (PNGData.Num() <= 0)
	{
		return;
	}


	// --------------------------------------------------------
	// multipart/form-data
	// --------------------------------------------------------

	const FString Boundary =
		TEXT(
			"----SafeStreetBoundary7MA4YWxkTrZu0gW"
		);

	const FString LineBreak =
		TEXT("\r\n");


	FString Header;

	Header += TEXT("--");
	Header += Boundary;
	Header += LineBreak;

	Header += TEXT(
		"Content-Disposition: form-data; "
		"name=\"file\"; filename=\"camera.png\""
	);

	Header += LineBreak;

	Header += TEXT(
		"Content-Type: image/png"
	);

	Header += LineBreak;
	Header += LineBreak;


	FTCHARToUTF8 HeaderUTF8(
		*Header
	);


	// IHttpRequest trabaja con TArray<uint8>.
	// El PNG generado por UE 5.5 es TArray64<uint8>,
	// por lo que construimos el cuerpo HTTP normal.
	TArray<uint8> RequestBody;


	const int64 EstimatedSize =
		static_cast<int64>(
			HeaderUTF8.Length()
			)
		+
		PNGData.Num()
		+
		256;


	if (EstimatedSize <= MAX_int32)
	{
		RequestBody.Reserve(
			static_cast<int32>(
				EstimatedSize
				)
		);
	}


	RequestBody.Append(
		reinterpret_cast<const uint8*>(
			HeaderUTF8.Get()
			),
		HeaderUTF8.Length()
	);


	RequestBody.Append(
		PNGData.GetData(),
		static_cast<int32>(
			PNGData.Num()
			)
	);


	FString Footer;

	Footer += LineBreak;
	Footer += TEXT("--");
	Footer += Boundary;
	Footer += TEXT("--");
	Footer += LineBreak;


	FTCHARToUTF8 FooterUTF8(
		*Footer
	);


	RequestBody.Append(
		reinterpret_cast<const uint8*>(
			FooterUTF8.Get()
			),
		FooterUTF8.Length()
	);


	// --------------------------------------------------------
	// Crear solicitud HTTP.
	// --------------------------------------------------------

	const FString ServerURL =
		TEXT(
			"http://127.0.0.1:8000/detect"
		);


	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();


	Request->SetURL(
		ServerURL
	);

	Request->SetVerb(
		TEXT("POST")
	);


	Request->SetHeader(
		TEXT("Content-Type"),
		FString::Printf(
			TEXT(
				"multipart/form-data; boundary=%s"
			),
			*Boundary
		)
	);


	Request->SetHeader(
		TEXT("Accept"),
		TEXT("application/json")
	);


	Request->SetContent(
		RequestBody
	);


	Request->OnProcessRequestComplete().BindUObject(
		this,
		&ASafeStreetAIClient::OnDetectionResponse
	);


	bAIRequestInProgress = true;


	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Enviando frame actual a SafeStreet AI..."
		)
	);


	if (!Request->ProcessRequest())
	{
		bAIRequestInProgress = false;

		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"No se pudo iniciar POST /detect."
			)
		);
	}
}


// ============================================================
// RESPUESTA DE DETECCION
// ============================================================

void ASafeStreetAIClient::OnDetectionResponse(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bWasSuccessful
)
{
	// --------------------------------------------------------
	// La solicitud termino.
	// Ya podemos permitir la siguiente inferencia.
	// --------------------------------------------------------

	bAIRequestInProgress = false;


	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"No se recibio respuesta de /detect."
			)
		);

		return;
	}


	const int32 ResponseCode =
		Response->GetResponseCode();

	const FString ResponseBody =
		Response->GetContentAsString();


	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"SafeStreet /detect - HTTP %d"
		),
		ResponseCode
	);


	if (ResponseCode != 200)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Error del servidor: %s"
			),
			*ResponseBody
		);

		return;
	}


	ParseDetectionResponse(
		ResponseBody
	);
}


// ============================================================
// INTERPRETAR JSON
// ============================================================

void ASafeStreetAIClient::ParseDetectionResponse(
	const FString& JsonResponse
)
{
	TSharedPtr<FJsonObject> RootObject;


	TSharedRef<TJsonReader<>> Reader =
		TJsonReaderFactory<>::Create(
			JsonResponse
		);


	if (
		!FJsonSerializer::Deserialize(
			Reader,
			RootObject
		)
		||
		!RootObject.IsValid()
		)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"No se pudo interpretar "
				"el JSON de la IA."
			)
		);

		return;
	}


	LastDetections.Empty();


	const int32 DetectionCount =
		RootObject->GetIntegerField(
			TEXT("detections_count")
		);


	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"========================================"
		)
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"     DETECCIONES CAMARA SAFESTREET"
		)
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"========================================"
		)
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Cantidad de detecciones: %d"
		),
		DetectionCount
	);


	const TArray<TSharedPtr<FJsonValue>>*
		DetectionsArray;


	if (
		!RootObject->TryGetArrayField(
			TEXT("detections"),
			DetectionsArray
		)
		)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"El JSON no contiene detections."
			)
		);

		return;
	}


	int32 DetectionIndex = 1;


	for (
		const TSharedPtr<FJsonValue>& DetectionValue :
		*DetectionsArray
		)
	{
		const TSharedPtr<FJsonObject>
			DetectionObject =
			DetectionValue->AsObject();


		if (!DetectionObject.IsValid())
		{
			continue;
		}


		FSafeStreetDetection Detection;


		Detection.ClassId =
			DetectionObject->GetIntegerField(
				TEXT("class_id")
			);


		Detection.ClassName =
			DetectionObject->GetStringField(
				TEXT("class_name")
			);


		Detection.Confidence =
			static_cast<float>(
				DetectionObject->GetNumberField(
					TEXT("confidence")
				)
				);


		const TSharedPtr<FJsonObject>*
			BBoxObject;


		if (
			DetectionObject->TryGetObjectField(
				TEXT("bbox"),
				BBoxObject
			)
			)
		{
			Detection.X1 =
				static_cast<float>(
					(*BBoxObject)->GetNumberField(
						TEXT("x1")
					)
					);


			Detection.Y1 =
				static_cast<float>(
					(*BBoxObject)->GetNumberField(
						TEXT("y1")
					)
					);


			Detection.X2 =
				static_cast<float>(
					(*BBoxObject)->GetNumberField(
						TEXT("x2")
					)
					);


			Detection.Y2 =
				static_cast<float>(
					(*BBoxObject)->GetNumberField(
						TEXT("y2")
					)
					);
		}


		LastDetections.Add(
			Detection
		);


		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[%d] Clase: %s | Confianza: %.2f%%"
			),
			DetectionIndex,
			*Detection.ClassName,
			Detection.Confidence * 100.0f
		);


		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"    BBox: "
				"(%.2f, %.2f) -> (%.2f, %.2f)"
			),
			Detection.X1,
			Detection.Y1,
			Detection.X2,
			Detection.Y2
		);


		DetectionIndex++;
	}


	// --------------------------------------------------------
	// RENDIMIENTO
	// --------------------------------------------------------

	const TSharedPtr<FJsonObject>*
		PerformanceObject;


	if (
		RootObject->TryGetObjectField(
			TEXT("performance"),
			PerformanceObject
		)
		)
	{
		const double InferenceMs =
			(*PerformanceObject)->GetNumberField(
				TEXT("inference_ms")
			);


		const double TotalMs =
			(*PerformanceObject)->GetNumberField(
				TEXT("total_ms")
			);


		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Inferencia IA: %.2f ms"
			),
			InferenceMs
		);


		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Tiempo servidor: %.2f ms"
			),
			TotalMs
		);
	}


	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"========================================"
		)
	);


	// --------------------------------------------------------
	// MENSAJE EN PANTALLA
	// --------------------------------------------------------

	if (GEngine)
	{
		const FString Message =
			FString::Printf(
				TEXT(
					"SafeStreet AI: %d detecciones"
				),
				LastDetections.Num()
			);


		GEngine->AddOnScreenDebugMessage(
			-1,
			1.5f,
			FColor::Green,
			Message
		);
	}
}