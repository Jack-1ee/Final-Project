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
        int width, height;
        fscanf(fpDim, "%d %d", &width, &height);

        // 3.建立Header

    }

}

