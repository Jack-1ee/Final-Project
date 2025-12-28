#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

// 2D DCT 函數 執行 8x8 DCT 轉換
//利用 DCT 分離性 Y = C * X * C^T
double dct_matrix[8][8];       // 存 C 矩陣
double dct_matrix_T[8][8];     // 存 C^T (轉置) 矩陣

// 1. 初始化 DCT 矩陣 (只在 main 一開始呼叫一次)
void init_dct_matrix() {
    for (int j = 0; j < 8; j++) {
        double alpha = (j == 0) ? (1.0 / sqrt(2.0)) : 1.0;
        for (int i = 0; i < 8; i++) {
            dct_matrix[j][i] = 0.5 * alpha * cos((2 * i + 1) * j * M_PI / 16.0);    // C[j][i] 公式
            dct_matrix_T[i][j] = dct_matrix[j][i];  // 順便存轉置矩陣 C^T
        }
    }
}

// 2. 矩陣乘法版 perform_DCT
// Output = C * Input * C^T
void perform_DCT(double input[8][8], double output[8][8]) {
    double temp[8][8] = {0}; // 中間產物

    // 第一步: Temp = C * Input (矩陣乘法)
    for (int i = 0; i < 8; i++) {       // Row of C
        for (int j = 0; j < 8; j++) {   // Col of Input
            double sum = 0.0;
            for (int k = 0; k < 8; k++) {
                sum += dct_matrix[i][k] * input[k][j];
            }
            temp[i][j] = sum;
        }
    }

    // 第二步: Output = Temp * C^T (矩陣乘法)
    for (int i = 0; i < 8; i++) {       // Row of Temp
        for (int j = 0; j < 8; j++) {   // Col of C^T
            double sum = 0.0;
            for (int k = 0; k < 8; k++) {
                sum += temp[i][k] * dct_matrix_T[k][j];
            }
            output[i][j] = sum;
        }
    }
}

// 寫量化表到TXT
void write_qt_txt(const char* filename, const int qt[8][8]) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Error opening QT output file");
        return;
    }

    // 跑雙層迴圈
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            fprintf(fp, "%d ", qt[i][j]);
        }
        fprintf(fp, "\n"); // 換行
    }
    
    fclose(fp);
}

int main(int argc, char *argv[]) {
    
    // 初始化DCT矩陣
    init_dct_matrix();

    // 檢查是否有指令
    if(argc <=2){
        printf("Usage: %s <method> <argc...>\n", argv[0]);
        return 1;
    }

    int method = atoi(argv[1]); // ASCII to integer

    // methood 0: BMP to TXT
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

        // 寫入尺寸資訊到dim
        fprintf(fpDim, "%d %d %d %d %d %d %d %d\n", 
            width, height, 
            fileHeader.bfSize, infoHeader.biSizeImage,
            infoHeader.biXPelsPerMeter, infoHeader.biYPelsPerMeter,
            infoHeader.biClrUsed, infoHeader.biClrImportant
        );

        // 3.計算padding
        int padding = (4 - (width *3) %4) %4;   // 實際byte數=width*3 padding補到4的倍數

        unsigned char pixel[3]; // 暫存RGB
        unsigned char padBuf[3];    // 讀padding

        // 4. 讀pixel資料
        for (int i = 0; i< height; i++){
            for (int j = 0; j< width; j++){
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

    else if (method == 1){
        if (argc != 13){
            printf("Usage: encoder 1 <bmp> <Qt_Y> <Qt_Cb> <Qt_Cr> <dim> <qF_Y> <qF_Cb> <qF_Cr> <eF_Y> <eF_Cb> <eF_Cr>\n");
            return 1;
        }
        
        // 1.輸出量化表
        write_qt_txt(argv[3], std_lum_qt);   // 亮度
        write_qt_txt(argv[4], std_chr_qt);   // 色度 Cb
        write_qt_txt(argv[5], std_chr_qt);   // 色度 Cr

        // 2.讀取圖片
        FILE *fpIn = fopen(argv[2], "rb");  // rb=read binary

        // 防呆 檔案不存在
        if (!fpIn){
        perror("Error opening input BMP"); 
        return 1;
        }

        BITMAPFILEHEADER fileHeader;
        BITMAPINFOHEADER infoHeader;

        // 複製二進位資料到變數
        fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpIn);
        fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpIn);

        int width = infoHeader.biWidth;
        int height = infoHeader.biHeight;
        int absHeight = abs(height); // 高度絕對值

        // 寫入8個參數到dim
        FILE *fpDim = fopen(argv[6], "w");

        fprintf(fpDim, "%d %d %d %d %d %d %d %d\n", 
            width, height, 
            fileHeader.bfSize, infoHeader.biSizeImage,
            infoHeader.biXPelsPerMeter, infoHeader.biYPelsPerMeter,
            infoHeader.biClrUsed, infoHeader.biClrImportant
        );
        
        fclose(fpDim);

        // 3.開啟輸出檔 Binary
        FILE *fpQF_Y = fopen(argv[7], "wb");
        FILE *fpQF_Cb= fopen(argv[8], "wb");
        FILE *fpQF_Cr= fopen(argv[9], "wb");
        FILE *fpEF_Y = fopen(argv[10], "wb");
        FILE *fpEF_Cb= fopen(argv[11], "wb");
        FILE *fpEF_Cr= fopen(argv[12], "wb");

        // 讀取圖片資料
        unsigned char *imgBuffer = (unsigned char *)malloc(width * absHeight * 3);  //要動態記憶體 指標

        unsigned char padBuf[3];    // 讀padding
        int padding = (4 - (width *3) %4) %4;

        for (int j = 0; j < absHeight; j++){
            // 讀有效像素
            fread(imgBuffer + (j * width *3), 1, width *3, fpIn);

            // 讀padding並丟棄
            if (padding >0){
                fread(padBuf, 1, padding, fpIn);
             }
        }

        fclose(fpIn);   // 關檔

        // 準備變數
        double blkY[8][8], blkCb[8][8], blkCr[8][8];    //空間 轉換前
        double dctY[8][8], dctCb[8][8], dctCr[8][8];    //頻率 轉換後
        double signal_power[3][64] = {0};
        double error_power[3][64] = {0};
        short qVal; // 量化整數
        float eVal; // 誤差浮點數

        printf("Method 1 Encoding...\n");

        // 4.RGB to YCbCr to DCT to Quantization
        // 8x8區塊處理
        for (int r = 0; r < absHeight; r += 8 ){
            for (int c = 0; c < width; c += 8){
                
                // 4.1.RGB to YCbCr
                for (int i = 0; i < 8; i++){
                    for (int j = 0; j < 8; j++){
                        int r_idx = (r + i < absHeight) ? (r + i) : (absHeight-1);  // 防止超出邊界 邊界延伸
                        int c_idx = (c + j < width) ? (c + j) : (width-1);
                        int idx = (r_idx * width + c_idx) * 3;

                        double B = imgBuffer[idx];  // 讀BGR
                        double G = imgBuffer[idx +1];
                        double R = imgBuffer[idx +2];

                        // 轉YCbCr公式
                        blkY[i][j] = ( 0.299 *R + 0.587 *G + 0.114 *B) - 128;    //將亮度中心移動到0
                        blkCb[i][j] = (-0.1687 *R - 0.3313 *G + 0.5 *B);   //Cb Cr中心本來就是0
                        blkCr[i][j] = ( 0.5 *R - 0.4187 *G - 0.0813 *B);
                    }
                }
                
                // 4.2.DCT
                perform_DCT(blkY, dctY);
                perform_DCT(blkCb, dctCb);
                perform_DCT(blkCr, dctCr);

                // 4.3.量化+寫檔
                // Y
                for (int i = 0; i < 8; i++){    // ROW
                    for (int j = 0; j < 8; j++){    // COLUMN
                        int k = i * 8 + j;  // 0-63

                        qVal = (short)round(dctY[i][j] / std_lum_qt[i][j]); //四捨五入
                        eVal = (float)dctY[i][j] - qVal * std_lum_qt[i][j];

                        fwrite(&qVal, 2, 1, fpQF_Y);    // 寫入量化值
                        fwrite(&eVal, 4, 1, fpEF_Y);

                        signal_power[0][k] += dctY[i][j]*dctY[i][j];    // 振幅平方功率
                        error_power[0][k] += eVal*eVal;
                    }
                }

                // Cb
                for (int i = 0; i < 8; i++){    // ROW
                    for (int j = 0; j < 8; j++){    // COLUMN
                        int k = i * 8 + j;  // 0-63

                        qVal = (short)round(dctCb[i][j] / std_chr_qt[i][j]); //四捨五入
                        eVal = (float)dctCb[i][j] - qVal * std_chr_qt[i][j];

                        fwrite(&qVal, 2, 1, fpQF_Cb);    // 寫入量化值
                        fwrite(&eVal, 4, 1, fpEF_Cb);

                        signal_power[1][k] += dctCb[i][j]*dctCb[i][j];    // 振幅平方功率
                        error_power[1][k] += eVal*eVal;
                    }
                }

                // Cr
                for (int i = 0; i < 8; i++){    // ROW
                    for (int j = 0; j < 8; j++){    // COLUMN
                        int k = i * 8 + j;  // 0-63

                        qVal = (short)round(dctCr[i][j] / std_chr_qt[i][j]); //四捨五入
                        eVal = (float)dctCr[i][j] - qVal * std_chr_qt[i][j];

                        fwrite(&qVal, 2, 1, fpQF_Cr);    // 寫入量化值
                        fwrite(&eVal, 4, 1, fpEF_Cr);

                        signal_power[2][k] += dctCr[i][j]*dctCr[i][j];    // 振幅平方功率
                        error_power[2][k] += eVal*eVal;
                    }
                }
            }
        }

        // 5.印出SQNR
            const char *chName[3] = {"Y", "Cb", "Cr"};
            for (int ch = 0; ch < 3; ch++){
                printf("Channel %s SQNR (dB):\n", chName[ch]);

                for(int k=0; k<64; k++){
                    double sqnr = (error_power[ch][k] == 0) ? 
                    999.9 : 10*log10(signal_power[ch][k] / error_power[ch][k]);   //999.9表示無限大
                
                    printf("%6.2f ", sqnr); //小數點後兩位

                    //換行
                    if((k+1) %8 == 0){
                        printf("\n");
                    }
                }
            }        
              
        // 6.關檔
        free(imgBuffer);
        fclose(fpQF_Y); fclose(fpQF_Cb); fclose(fpQF_Cr);
        fclose(fpEF_Y); fclose(fpEF_Cb); fclose(fpEF_Cr);
        printf("Method 1 Encoding Done.\n");
    }
    
    return 0;
}