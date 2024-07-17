// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Vmd.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Animation/SkeletalMeshActor.h"
#include "MyActor.generated.h"

UCLASS()
class UE_MMD_CAMERALOAD_API AMyActor : public AActor
{
	GENERATED_BODY()
private:
	/// <summary>
	/// フレーム補間用フラグ。trueにすると有効になる
	/// あまり見映えが変わらなかったのでデフォルトでは切っている
	/// </summary>
	bool enable_frame_interpolate = false;
	/// <summary>
	/// GetPos用最終処理フレーム記憶変数
	/// </summary>
	int last_processed_frame_for_getpos = -1;
	int last_processed_frame_for_getrot = -1;
	int last_processed_frame_for_fov = -1;
	int last_processed_frame_for_distance = -1;
	int n_frame;
	frame_t* frames;
	void Load(const char* path);
public:	
	// Sets default values for this actor's properties
	AMyActor();
	UFUNCTION(BlueprintCallable)
	FVector GetPos(float Time);
	UFUNCTION(BlueprintCallable)
	FRotator GetRot(float Time);
	UFUNCTION(BlueprintCallable)
	float GetFov(float Time);
	UFUNCTION(BlueprintCallable)
	float GetDistance(float Time);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	float ConvertVerticalFOVToHorizontal(float VerticalFOV) const;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "My Variables")
	FString VmdPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	ASkeletalMeshActor* MMDModel;
};
