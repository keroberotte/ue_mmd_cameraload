// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <stdint.h>
#include <stdio.h>

//ヘッダーの桁数
#define HEADER 50
//モーションレコード数
#define MOTIONCOUNT 4
//スキンレコード数
#define SKINCOUNT 4
#define FRAME_SIZE 61

extern "C" {
    typedef struct {
        float x, y, z;
    } vector3_t;

    typedef struct {
        int no;
        float distance;
        vector3_t pos;
        vector3_t rot;
        uint8_t bezier[24];
        float view_angle;
        uint8_t perspective;
        uint8_t is_original_frame;
    } frame_t;

    float BezierCurve(float v1, float v2, float v3, float v4, float t);

    float ConversionAngle(float rot);

    void LoadVmdCameraFrames(FILE* fp, int n_frame, frame_t* frames);

    void InterpolateVmdCameraFrames(
        int n_original_frame,
        int n_interpolate_frame,
        const frame_t* original_frames,
        frame_t* interpolated_frames
    );

    void DumpVmdCameraFrames(const char* path, int n_frame, frame_t* frames);

}