#include "vmd.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M_PI 3.141592

static void Qsort(frame_t* data, int left, int right)
{
    int i, j;
    int pivot;
    frame_t tmp;

    i = left; j = right;
    pivot = data[(left + right) / 2].no;
    do
    {
        while ((i < right) && (data[i].no < pivot)) i++;
        while ((j > left) && (pivot < data[j].no)) j--;
        if (i <= j)
        {
            tmp = data[i];
            data[i] = data[j];
            data[j] = tmp;
            i++; j--;
        }
    } while (i <= j);
    if (left < j) Qsort(data, left, j);
    if (i < right) Qsort(data, i, right);
}

float BezierCurve(float v1, float v2, float v3, float v4, float t) {
    return powf(1 - t, 3) * v1 +
        3 * powf(1 - t, 2) * t * v2 +
        3 * (1 - t) * powf(t, 2) * v3 +
        powf(t, 3) * v4;
}

float ConversionAngle(float rot) {
    return rot * 180 / M_PI;
}

/// <summary>
/// 
/// </summary>
/// <param name="fp"></param>
/// <param name="n_frame"></param>
/// <param name="frames"></param>
void LoadVmdCameraFrames(FILE* fp, int n_frame, frame_t* frames) {
    for (int i = 0; i < n_frame; i++) {
        fread((void*)(&frames[i]), 1, FRAME_SIZE, fp);
        // int型の値がfloat変数に入った状態になっているので、intに変換したうえでfloatにキャストする
        frames[i].view_angle = *(int*)(&frames[i].view_angle);
        frames[i].rot.x = ConversionAngle(frames[i].rot.x);
        frames[i].rot.y = ConversionAngle(frames[i].rot.y);
        frames[i].rot.z = ConversionAngle(frames[i].rot.z);
    }
    Qsort(frames, 0, n_frame - 1);
}

/// <summary>
/// 
/// </summary>
/// <param name="n_original_frame"></param>
/// <param name="n_interpolate_frame"></param>
/// <param name="original_frames"></param>
/// <param name="interpolated_frames"></param>
void InterpolateVmdCameraFrames(
    int n_original_frame,
    int n_interpolate_frame,
    const frame_t* original_frames,
    frame_t* interpolated_frames
) {
    for (int i = 0; i < n_original_frame - 1; i++) {
        interpolated_frames[original_frames[i].no] = original_frames[i];
        interpolated_frames[original_frames[i+1].no] = original_frames[i+1];
        interpolated_frames[original_frames[i].no].is_original_frame = 1;
        interpolated_frames[original_frames[i+1].no].is_original_frame = 1;
        int add_frame = original_frames[i+1].no - original_frames[i].no;
        for (int j = 1; j < add_frame; j++) {
            interpolated_frames[original_frames[i].no + j].no = original_frames[i].no + j;
            interpolated_frames[original_frames[i].no + j].is_original_frame = 0;
            // pos
            interpolated_frames[original_frames[i].no + j].pos.x = original_frames[i].pos.x + (original_frames[i+1].pos.x - original_frames[i].pos.x) * BezierCurve(
                0, original_frames[i+1].bezier[2], original_frames[i+1].bezier[3], 127, j / (float)add_frame
            ) / 127.0;
            interpolated_frames[original_frames[i].no + j].pos.y = original_frames[i].pos.y + (original_frames[i+1].pos.y - original_frames[i].pos.y) * BezierCurve(
                0, original_frames[i+1].bezier[6], original_frames[i+1].bezier[7], 127, j / (float)add_frame
            ) / 127.0;
            interpolated_frames[original_frames[i].no + j].pos.z = original_frames[i].pos.z + (original_frames[i+1].pos.z - original_frames[i].pos.z) * BezierCurve(
                0, original_frames[i+1].bezier[10], original_frames[i+1].bezier[11], 127, j / (float)add_frame
            ) / 127.0;
            // rot
            interpolated_frames[original_frames[i].no + j].rot.x = original_frames[i].rot.x + (original_frames[i+1].rot.x - original_frames[i].rot.x) * BezierCurve(
                0, original_frames[i+1].bezier[14], original_frames[i+1].bezier[15], 127, j / (float)add_frame
            ) / 127.0;
            interpolated_frames[original_frames[i].no + j].rot.y = original_frames[i].rot.y + (original_frames[i+1].rot.y - original_frames[i].rot.y) * BezierCurve(
                0, original_frames[i+1].bezier[14], original_frames[i+1].bezier[15], 127, j / (float)add_frame
            ) / 127.0;
            interpolated_frames[original_frames[i].no + j].rot.z = original_frames[i].rot.z + (original_frames[i+1].rot.z - original_frames[i].rot.z) * BezierCurve(
                0, original_frames[i+1].bezier[14], original_frames[i+1].bezier[15], 127, j / (float)add_frame
            ) / 127.0;
            // distance
            interpolated_frames[original_frames[i].no + j].distance = original_frames[i].distance + (original_frames[i+1].distance - original_frames[i].distance) * BezierCurve(
                0, original_frames[i+1].bezier[18], original_frames[i+1].bezier[19], 127, j / (float)add_frame
            ) / 127.0;
            // view angle
            interpolated_frames[original_frames[i].no + j].view_angle = original_frames[i].view_angle + (original_frames[i+1].view_angle - original_frames[i].view_angle) * (int)(BezierCurve(
                0, original_frames[i+1].bezier[22], original_frames[i+1].bezier[23], 127, j / (float)add_frame
            )) / 127.0;
        }
    }
}

/// <summary>
/// 受け取ったフレーム情報をcsvファイルにダンプする。デバッグ用
/// </summary>.
/// <param name="path">出力ファイルパス</param>
/// <param name="n_frame">データ長</param>
/// <param name="frames">フレームデータ</param>
void DumpVmdCameraFrames(const char* path, int n_frame, frame_t* frames) {
    FILE* wfp = fopen(path, "w");
    char line[1024];
    sprintf(line, "frame,distance,pos_x,pos_y,pos_z,rot_x,rot_y,rot_z,view_angle,");
    fwrite(line, 1, strlen(line), wfp);
    for (int i = 0; i < 24; i++) {
        sprintf(line, "bezier%d,", i);
        fwrite(line, 1, strlen(line), wfp);
    }
    sprintf(line, "is_original_frame\n");
    fwrite(line, 1, strlen(line), wfp);
    for (int i = 0; i < n_frame; i++) {
        frame_t f = frames[i];
        sprintf(line, "%d,%f,%f,%f,%f,%f,%f,%f,%f,", f.no, f.distance, f.pos.x, f.pos.y, f.pos.z, f.rot.x, f.rot.y, f.rot.z, f.view_angle);
        fwrite(line, 1, strlen(line), wfp);
        for (int j = 0; j < 24; j++) {
            if (f.is_original_frame) {
                sprintf(line, "%d,", f.bezier[j]);
            }
            else {
                sprintf(line, ",");
            }
            fwrite(line, 1, strlen(line), wfp);
        }
        sprintf(line, "%d\n", f.is_original_frame);
        fwrite(line, 1, strlen(line), wfp);
    }
    fclose(wfp);
}

