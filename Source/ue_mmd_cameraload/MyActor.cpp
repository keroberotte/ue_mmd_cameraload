// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"
#include "Vmd.h"
#include "Kismet/KismetSystemLibrary.h"

void AMyActor::Load(const char* path) {
	FILE* fp = fopen(path, "rb");
	fseek(fp, HEADER + MOTIONCOUNT + SKINCOUNT, SEEK_SET);
	int n_original_frame;
	fread(&n_original_frame, 4, 1, fp);
	frame_t* original_frames = new frame_t[n_original_frame];
	LoadVmdCameraFrames(fp, n_original_frame, original_frames);
	fclose(fp);
	n_frame = original_frames[n_original_frame - 1].no + 1;
	frames = new frame_t[n_frame];
	InterpolateVmdCameraFrames(n_original_frame, n_frame, original_frames, frames);
	delete[] original_frames;
}

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	Load(TCHAR_TO_ANSI(*VmdPath));
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float interpolate(float next_v, float current_v, float t) {
	float delta = t - (int)t;
	return current_v + (next_v - current_v) * delta;
}

vector3_t interpolate(vector3_t next_v, vector3_t current_v, float t) {
	vector3_t v;
	v.x = interpolate(next_v.x, current_v.x, t);
	v.y = interpolate(next_v.y, current_v.y, t);
	v.z = interpolate(next_v.z, current_v.z, t);
	return v;
}

/// <summary>
/// frame_noに対応する位置情報を返却する
/// </summary>
/// <param name="time"></param>
/// <returns></returns>
FVector AMyActor::GetPos(float frame_no) {
	if (!enable_frame_interpolate || frame_no >= n_frame || last_processed_frame_for_getpos != (int)frame_no) {
		// Unityのx, y, z軸はUnreal Engineのy, x, z軸と対応するため、軸の入れ替えを行なう
		return FVector(
			frames[(int)frame_no].pos.z,
			frames[(int)frame_no].pos.x,
			frames[(int)frame_no].pos.y
		);
	}
	vector3_t v = interpolate(frames[(int)frame_no + 1].pos, frames[(int)frame_no].pos, frame_no);
	last_processed_frame_for_getpos = (int)frame_no;
	return FVector(
		v.z,
		v.x,
		v.y
	);
}

FRotator AMyActor::GetRot(float frame_no) {
	if (!enable_frame_interpolate || frame_no >= n_frame || last_processed_frame_for_getrot != (int)frame_no) {
		// Unityのx, y, z軸はUnreal Engineのy, x, z軸と対応するため、軸の入れ替えを行なう
		// 回転については厳密に理解したわけではないが、見た目としては近いものになっている
		FRotator ret = FRotator::MakeFromEuler(FVector(
			frames[(int)frame_no].rot.z,
			frames[(int)frame_no].rot.x,
			frames[(int)frame_no].rot.y
		));
		ret.Yaw *= -1;
		return ret;
	}
	vector3_t v = interpolate(frames[(int)frame_no + 1].rot, frames[(int)frame_no].rot, frame_no);
	FRotator ret = FRotator::MakeFromEuler(FVector(
		v.z,
		v.x,
		v.y
	));
	ret.Yaw *= -1;
	last_processed_frame_for_getrot = (int)frame_no;
	return ret;
}

float AMyActor::GetFov(float frame_no) {
	if (!enable_frame_interpolate || frame_no >= n_frame || last_processed_frame_for_fov != (int)frame_no) {
		return ConvertVerticalFOVToHorizontal(frames[(int)frame_no].view_angle);
	}
	last_processed_frame_for_fov = (int)frame_no;
	// Unityは垂直視野角、Unreal Engineは水平視野角を用いているため、変換を行なう
	return interpolate(ConvertVerticalFOVToHorizontal(frames[(int)frame_no + 1].view_angle), ConvertVerticalFOVToHorizontal(frames[(int)frame_no].view_angle), frame_no);
}

float AMyActor::GetDistance(float frame_no) {
	if (!enable_frame_interpolate || frame_no >= n_frame || last_processed_frame_for_distance != (int)frame_no) {
		return frames[(int)frame_no].distance;
	}
	last_processed_frame_for_distance = (int)frame_no;
	return interpolate(frames[(int)frame_no + 1].distance, frames[(int)frame_no].distance, frame_no);
}

/// <summary>
/// 垂直視野角から水平視野角への変換を行なう
/// </summary>
/// <param name="VerticalFOV">垂直視野角（degree）</param>
/// <returns>水平視野角（degree）</returns>
float AMyActor::ConvertVerticalFOVToHorizontal(float VerticalFOV) const
{
	// Convert Vertical FOV to Horizontal FOV
	const float AspectRatio = GEngine->GameViewport->Viewport->GetDesiredAspectRatio();
	float VerticalFOVRadians = FMath::DegreesToRadians(VerticalFOV);
	float HorizontalFOVRadians = 2.0f * FMath::Atan(FMath::Tan(VerticalFOVRadians / 2.0f) * AspectRatio);
	return FMath::RadiansToDegrees(HorizontalFOVRadians);
}