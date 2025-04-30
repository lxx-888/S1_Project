#ifndef DMS_IPC_H
#define DMS_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

#define DMS_LANDMARK_POINTS 66

typedef struct{
    bool isSmoke;
    bool isPhone;
    bool isTired;
    bool isDistracted;
    bool isYawn;
} STATUS;

typedef struct _dms_point {
    float x;
    float y;
} dms_point;

typedef struct _dms_face_rect{
    float x;
    float y;
    float width;
    float height;
    float detWidth;
    float detHeight;
} dms_face_rect;

#ifdef __cplusplus
}
#endif

#endif