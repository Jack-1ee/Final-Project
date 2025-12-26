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

    // method 0: encode
    if (method == 0){
        if (argc != 7){
            printf("Usage: encoder 0 <input.bmp> <R.txt> <G.txt> <B.txt> <dim.txt>\n");
        return 1;
        }

        // 1.讀檔
        FILE *fpIn = fopen(argv[2], "rb");  // rb=read binary
    
        // 防呆 檔案不存在
        if (!fpIn){
        perror("Error opening input BMP"); 
        return 1;
        }
    
        // w=write
        FILE *fpR  = fopen(argv[3], "w");
        FILE *fpG  = fopen(argv[4], "w");
        FILE *fpB  = fopen(argv[5], "w");
        FILE *fpDim= fopen(argv[6], "w");

        // 2.讀Header
        BITMAPFILEHEADER fileHeader;
        BITMAPINFOHEADER infoHeader;

        // 複製二進位資料到變數
        fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpIn);
        fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpIn);

        int width = infoHeader.biWidth;
        int height = infoHeader.biHeight;

        fprintf(fpDim, "%d %d\n", width, height);    // 寫入尺寸資訊

        // 3.計算padding
        int padding = (4 - (width *3) %4) %4;   // 實際byte數=width*3 padding補到4的倍數

        unsigned char pixel[3]; // 暫存RGB
        unsigned char padBuf[3];    // 讀padding

        // 4. 讀pixel資料
        for (int i=0; i< height; i++){
            for (int j=0; j< width; j++){
                fread(pixel, 1, 3, fpIn);   // 讀3個byte到pixel
                fprintf(fpR, "%d ", pixel[2]);  // R
                fprintf(fpG, "%d ", pixel[1]);  // G
                fprintf(fpB, "%d ", pixel[0]);  // B
            }

            // 換行
            fprintf(fpR, "\n");
            fprintf(fpG, "\n");
            fprintf(fpB, "\n");

            // 5.處理padding
            if (padding >0){
                fread(padBuf, 1, padding, fpIn);
            }
        }

    fclose(fpIn);   // 關檔
    fclose(fpR);
    fclose(fpG);
    fclose(fpB);
    fclose(fpDim);
    
    printf("Method 0 Encoding Done.\n");
    }

    return 0;
}