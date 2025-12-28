#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

int main(int argc, char *argv[]) {
    
    // 檢查是否有指令
    if(argc <=2){
        printf("Usage: %s <method> <argc...>\n", argv[0]);
        return 1;
    }

    int method = atoi(argv[1]); // ASCII to integer
    
    // method 0: TXT to BMP
    if (method == 0){
        if (argc != 7){
            printf("Error: Method 0 needs 6 arguments.\n");
        return 1;
        }

        // 1.建檔
        FILE *fpOut = fopen(argv[2], "wb");  // wb=write binary
        FILE *fpR  = fopen(argv[3], "r");
        FILE *fpG  = fopen(argv[4], "r");
        FILE *fpB  = fopen(argv[5], "r");
        FILE *fpDim= fopen(argv[6], "r");

        // 檢查檔案是否成功開啟
        if (!fpOut || !fpR || !fpG || !fpB || !fpDim){
            perror("Error opening files"); 
            return 1;
        }

        // 2.讀取尺寸
        int width, height, bfSize, biSizeImage, xPels, yPels, clrUsed, clrImp;
        fscanf(fpDim, "%d %d %d %d %d %d %d %d", 
            &width, 
            &height, 
            &bfSize,      // 檔案大小
            &biSizeImage, // 影像資料大小
            &xPels,   // X 解析度
            &yPels,   // Y 解析度
            &clrUsed, // 使用顏色數
            &clrImp   // 重要顏色數
            );

        // 4.寫Header
        int padding = (4 - (width *3) %4) %4;

        BITMAPFILEHEADER fileHeader = {0};  // 歸0
            fileHeader.bfType = 0x4D42; // "BM"
            fileHeader.bfSize = bfSize;
            fileHeader.bfOffBits = 54;  // Header 14+40

        BITMAPINFOHEADER infoHeader = {0};  // 歸0
            infoHeader.biSize = 40;
            infoHeader.biWidth = width;
            infoHeader.biHeight = height;
            infoHeader.biPlanes = 1;
            infoHeader.biBitCount = 24; // RGB 3byte=24bits
            infoHeader.biSizeImage = biSizeImage;
            infoHeader.biXPelsPerMeter = xPels;
            infoHeader.biYPelsPerMeter = yPels;
            infoHeader.biClrUsed = clrUsed;
            infoHeader.biClrImportant = clrImp;
        
        // 寫入Header
        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpOut);
        fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpOut);

        // 5.讀TXT寫入BMP
        int rVal, gVal, bVal;   // 用來fscanf
        unsigned char pixel[3]; // 用來fwrite
        unsigned char padBuf[3] = {0, 0, 0};    // Padding 全0

        for ( int i = 0; i < height; i++){
            for (int j = 0; j < width; j++){
                fscanf(fpR, "%d", &rVal);   // 文字檔讀數值
                fscanf(fpG, "%d", &gVal);   
                fscanf(fpB, "%d", &bVal);
                
                pixel[0] = (unsigned char)bVal; // 轉回BGR順序
                pixel[1] = (unsigned char)gVal;
                pixel[2] = (unsigned char)rVal;

                fwrite(pixel, 1, 3, fpOut); // 寫3個byte
            }
            // 寫入Padding
            if (padding > 0){
                fwrite(padBuf, 1, padding, fpOut);
            } 
        }
        
        printf("Method 0 Decoding Done. Output: %s\n", argv[2]);

        fclose(fpOut);  // 關檔
        fclose(fpR);
        fclose(fpG);
        fclose(fpB);
        fclose(fpDim);
    }

    return 0;
}

