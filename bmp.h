#ifndef BMP_H
#define BMP_H

#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 不要自動對齊 1-byte
#pragma pack(push, 1)

// BMPFILEHEADER
typedef struct {
    uint16_t bfType;        // BMP檔標誌BM 0x4D42 (ASII碼)      
    uint32_t bfSize;        // BMP檔大小 (Header+Data)
    uint16_t bfReserved1;   // 保留字 0
    uint16_t bfReserved2;   // 保留字 0
    uint32_t bfOffBits;     // 檔案開頭到位圖資料的偏移量 (14+40=54)
}BITMAPFILEHEADER;

// BMPINFOHEADER
typedef struct {
    uint32_t biSize;            // 本結構大小 40
    int32_t biWidth;            // 圖片寬度
    int32_t biHeight;           // 圖片高度 +倒著存 -正著存
    uint16_t biPlanes;          // 平面數 1
    uint16_t biBitCount;        // 每像素位元數 RGB=24
    uint32_t biCompression;     // 壓縮類型 0=不壓縮
    uint32_t biSizeImage;       // 圖片資料大小 (RowSize+Padding)*Height
    int32_t biXPelsPerMeter;    // 水平解析度 (像素/公尺)
    int32_t biYPelsPerMeter;    // 垂直解析度 (像素/公尺)
    uint32_t biClrUsed;         // 使用的色彩數 0=預設值
    uint32_t biClrImportant;    // 重要的色彩數 0=全部重要
}BITMAPINFOHEADER;

// 恢復自動對齊
#pragma pack(pop)

// 標準 JPEG 量化表 亮度 Y
static const int std_lum_qt[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}
};

// 標準 JPEG 量化表 色度 Cb/Cr
static const int std_chr_qt[8][8] = {
    {17, 18, 24, 47, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}
};

// ZigZag 掃描順序 (8x8 -> 1x64)
static const int zigzag_order[64] = {
    0,  1,  5,  6, 14, 15, 27, 28,
    2,  4,  7, 13, 16, 26, 29, 42,
    3,  8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
   10, 19, 23, 32, 39, 45, 52, 54,
   20, 22, 33, 38, 46, 51, 55, 60,
   21, 34, 37, 47, 50, 56, 59, 61,
   35, 36, 48, 49, 57, 58, 62, 63
};

#endif // BPM_H