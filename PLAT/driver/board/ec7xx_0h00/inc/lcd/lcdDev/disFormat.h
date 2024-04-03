#ifndef  _DIS_FMT_H_
#define  _DIS_FMT_H_
#ifdef __cplusplus
extern "C" {
#endif
void yuv422ToRgb565(const void *inbuf, void *outbuf, int width, int height);
void yuv420ToRgb565(const void* inbuf, void* outbuf, int width, int height);
void rgb565ToYuv422(const void *inbuf, void *outbuf, int width, int height);
void rgb565ToYuv420(const void* inbuf, void* outbuf, int width, int height);
void rgb565ToYCbCr(const void *inbuf, void *outbuf, int width, int height);
void yCbCrToRgb565(const void *inbuf, void *outbuf, int width, int height);
#ifdef __cplusplus
}
#endif
#endif /* _DIS_FMT_H_ */

