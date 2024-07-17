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

FVector AMyActor::GetPos(float time) {
	if (!enable_frame_interpolate || time >= n_frame || last_processed_frame1 != (int)time) {
		return FVector(
			frames[(int)time].pos.z,
			frames[(int)time].pos.x,
			frames[(int)time].pos.y
		);
	}
	vector3_t v = interpolate(frames[(int)time + 1].pos, frames[(int)time].pos, time);
	last_processed_frame1 = (int)time;
	return FVector(
		v.z,
		v.x,
		v.y
	);
}

FRotator AMyActor::GetRot(float time) {
	if (!enable_frame_interpolate || time >= n_frame || last_processed_frame2 != (int)time) {
		FRotator ret = FRotator::MakeFromEuler(FVector(
			frames[(int)time].rot.z,
			frames[(int)time].rot.x,
			frames[(int)time].rot.y
		));
		ret.Yaw *= -1;
		return ret;
	}
	vector3_t v = interpolate(frames[(int)time + 1].rot, frames[(int)time].rot, time);
	FRotator ret = FRotator::MakeFromEuler(FVector(
		v.z,
		v.x,
		v.y
	));
	ret.Yaw *= -1;
	last_processed_frame2 = (int)time;
	return ret;
}

float AMyActor::GetFov(float time) {
	if (!enable_frame_interpolate || time >= n_frame || last_processed_frame3 != (int)time) {
		return ConvertVerticalFOVToHorizontal(frames[(int)time].view_angle);
	}
	last_processed_frame3 = (int)time;
	return interpolate(ConvertVerticalFOVToHorizontal(frames[(int)time + 1].view_angle), ConvertVerticalFOVToHorizontal(frames[(int)time].view_angle), time);
}

float AMyActor::GetDistance(float time) {
	if (!enable_frame_interpolate || time >= n_frame || last_processed_frame4 != (int)time) {
		return frames[(int)time].distance;
	}
	last_processed_frame4 = (int)time;
	return interpolate(frames[(int)time + 1].distance, frames[(int)time].distance, time);
}

float AMyActor::ConvertVerticalFOVToHorizontal(float VerticalFOV) const
{
	// Convert Vertical FOV to Horizontal FOV
	const float AspectRatio = GEngine->GameViewport->Viewport->GetDesiredAspectRatio();
	float VerticalFOVRadians = FMath::DegreesToRadians(VerticalFOV);
	float HorizontalFOVRadians = 2.0f * FMath::Atan(FMath::Tan(VerticalFOVRadians / 2.0f) * AspectRatio);
	return FMath::RadiansToDegrees(HorizontalFOVRadians);
}