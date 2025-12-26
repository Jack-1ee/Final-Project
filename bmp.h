#ifndef BMP_H
#define BMP_H

#include <stdint.h>
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

#endif // BPM_H