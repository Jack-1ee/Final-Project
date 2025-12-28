#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

// 控制範圍0-255
#define CLAMP(x) (((x) < 0) ? 0 : (((x) > 255) ? 255 : (unsigned char)((x) + 0.5)))

// 定義變數DCT 矩陣
double dct_matrix[8][8];       // C
double dct_matrix_T[8][8];     // C^T

// 1.初始化 IDCT 矩陣
// 先算好cos
void init_dct_matrix() {
    for (int j = 0; j < 8; j++) {
        double alpha = (j == 0) ? (1.0 / sqrt(2.0)) : 1.0;
        for (int i = 0; i < 8; i++) {
            dct_matrix[j][i] = 0.5 * alpha * cos((2 * i + 1) * j * M_PI / 16.0);
            dct_matrix_T[i][j] = dct_matrix[j][i];
        }
    }
}

// 2.分離性 IDCT (Inverse DCT)
// Output = C^T * Input * C
void perform_IDCT(double input[8][8], double output[8][8]) {
    double temp[8][8] = {0};

    // 2.1.Temp = C^T * Input (注意這裡是乘 C^T)
    for (int i = 0; i < 8; i++) {       // Row of C^T
        for (int j = 0; j < 8; j++) {   // Col of Input
            double sum = 0.0;
            for (int k = 0; k < 8; k++) {
                sum += dct_matrix_T[i][k] * input[k][j];
            }
            temp[i][j] = sum;
        }
    }

    // 2.2.Output = Temp * C
    for (int i = 0; i < 8; i++) {       // Row of Temp
        for (int j = 0; j < 8; j++) {   // Col of C
            double sum = 0.0;
            for (int k = 0; k < 8; k++) {
                sum += temp[i][k] * dct_matrix[k][j];
            }
            output[i][j] = sum;
        }
    }
}

// 讀取量化表
void read_qt_txt(const char* filename, int qt[8][8]) {
    FILE* fp = fopen(filename, "r");
    if (!fp) { perror("Error opening QT file"); exit(1); }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            fscanf(fp, "%d", &qt[i][j]);    // fp寫入qt[i][j]
        }
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

        // 3.寫Header
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
        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpOut);    // Header寫入檔案 
        fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpOut);

        // 4.讀TXT寫入BMP
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

    else if (method == 1){
        int is_lossless = 0;    // 0=有損(1a) 1=無損(1b)
        char *outBmpFile, *refBmpFile;  // 輸出檔BMP 原始檔BMP
        char *qtYFile, *qtCbFile, *qtCrFile, *dimFile;  // 量化表檔
        char *qfYFile, *qfCbFile, *qfCrFile;    // 量化值檔
        char *efYFile = NULL, *efCbFile = NULL, *efCrFile = NULL;   // 誤差值檔 無損才讀

        // 判斷有損(1a)或無損(1b)
        // argc=11 有損
        if (argc == 11){
            printf("Mode 1(a): Lossy Reconstruction & SQNR check\n");
            outBmpFile = argv[2];
            refBmpFile = argv[3]; // 原始圖檔
            qtYFile = argv[4]; qtCbFile = argv[5]; qtCrFile = argv[6];
            dimFile = argv[7];
            qfYFile = argv[8]; qfCbFile = argv[9]; qfCrFile = argv[10];
        }
        
        // argc=13 無損
        else if (argc == 13){
            printf("Mode 1(b): Lossless Reconstruction\n");
            is_lossless = 1;
            outBmpFile = argv[2];
            refBmpFile = NULL;  // 無需原始圖檔
            qtYFile = argv[3]; qtCbFile = argv[4]; qtCrFile = argv[5];
            dimFile = argv[6];
            qfYFile = argv[7]; qfCbFile = argv[8]; qfCrFile = argv[9];
            efYFile = argv[10]; efCbFile = argv[11]; efCrFile = argv[12];   //無損多加error files
        }
        
        else{
            printf("Error: method 1 arguments count mismatch (needs 11 or 13).\n");
            return 1;
        }

        // 1.讀取dim.txt重建Header
        FILE *fpDim = fopen(dimFile, "r");

        if (!fpDim){
            perror("Open dim error"); 
            return 1;
        }

        // 讀取尺寸
        int width, height, bfSize, biSizeImage, xpels, ypels, clrUsed, clrImp;
        fscanf(fpDim, "%d %d %d %d %d %d %d %d", 
            &width,
            &height, 
            &bfSize, 
            &biSizeImage, 
            &xpels, 
            &ypels, 
            &clrUsed, 
            &clrImp
        );

        fclose(fpDim);

        int absHeight = abs(height);    // 高度絕對值
        int padding = (4 - (width * 3) % 4) % 4;    // Padding大小

        // 2.讀取量化表
        int qtY[8][8], qtCb[8][8], qtCr[8][8];
        read_qt_txt(qtYFile, qtY);
        read_qt_txt(qtCbFile, qtCb);
        read_qt_txt(qtCrFile, qtCr);

        // 3.寫入RAW
        FILE *fqY = fopen(qfYFile, "rb");   // 開檔Read Binary
        FILE *fqCb = fopen(qfCbFile, "rb");
        FILE *fqCr = fopen(qfCrFile, "rb");

        FILE *feY = NULL, *feCb = NULL, *feCr = NULL;   // 避免有損模式變亂碼
        
        // 無損模式才讀誤差值檔
        if(is_lossless) {
            feY = fopen(efYFile, "rb");
            feCb = fopen(efCbFile, "rb");
            feCr = fopen(efCrFile, "rb");
        }

        if (!fqY || !fqCb || !fqCr) { 
            perror("Error opening .raw files"); 
            return 1; 
        }

        unsigned char *imgBuffer = (unsigned char*)malloc(width * absHeight * 3);   // 動態記憶體

        // 4.解迴圈重建影像
        double blkY[8][8], blkCb[8][8], blkCr[8][8];    // 反離散餘弦
        double dctY[8][8], dctCb[8][8], dctCr[8][8];    // 反量化

        short qVal; // 量化值
        float eVal; // 誤差值

        for (int r = 0; r < absHeight; r += 8){
            
            // 每 100 行印一次進度，不然會以為當機
            if (r % 100 == 0) {
                printf("Decoding Row: %d / %d\n", r, absHeight);
            }
            
            for (int c = 0; c < width; c +=8 ){
                
                // Y
                for (int i = 0; i < 8; i++){
                    for (int j = 0; j < 8; j++){
                        fread(&qVal, 2, 1, fqY);    // 讀量化值
                        eVal = 0.0; // 先預設誤差=0
                        if(is_lossless){
                            fread(&eVal, 4, 1, feY); 
                        }

                        dctY[i][j] = (double)qVal * qtY[i][j] + eVal;   // 反量化公式 量化回DCT
                    }  
                }
                perform_IDCT(dctY, blkY);   // 反DCT DCT回YCbCr

                // Cb
                for (int i = 0; i < 8; i++){
                    for (int j = 0; j < 8; j++){
                        fread(&qVal, 2, 1, fqCb);    // 讀量化值
                        eVal = 0.0;
                        if(is_lossless){
                            fread(&eVal, 4, 1, feCb); 
                        }

                        dctCb[i][j] = (double)qVal * qtCb[i][j] + eVal;   // 反量化公式 量化回DCT
                    }
                }
                perform_IDCT(dctCb, blkCb);  // 反DCT DCT回YCbCr
                
                // Cr
                for (int i = 0; i < 8; i++){
                    for (int j = 0; j < 8; j++){
                        fread(&qVal, 2, 1, fqCr);    // 讀量化值
                        eVal = 0.0;
                        if(is_lossless){
                            fread(&eVal, 4, 1, feCr); 
                        }

                        dctCr[i][j] = (double)qVal * qtCr[i][j] + eVal;   // 反量化公式 量化回DCT
                    }
                }
                perform_IDCT(dctCr, blkCr); // 反DCT DCT回YCbCr

                // 5.YCbCr回RGB
                for(int i=0; i<8; i++){
                    for(int j=0; j<8; j++){
                        int r_pos = r + i;  //r是大方塊
                        int c_pos = c + j;  //c是大方塊
                        
                        if(r_pos < absHeight && c_pos < width) {
                            
                            double Y = blkY[i][j];
                            double Cb = blkCb[i][j];
                            double Cr = blkCr[i][j];

                            Y += 128.0; // 加回 128

                            // 轉回 RGB
                            double R = Y + 1.402 * Cr;
                            double G = Y - 0.344136 * Cb - 0.714136 * Cr;
                            double B = Y + 1.772 * Cb;

                            int idx = (r_pos * width + c_pos) * 3;
                            imgBuffer[idx]     = CLAMP(B);
                            imgBuffer[idx + 1] = CLAMP(G);
                            imgBuffer[idx + 2] = CLAMP(R);
                        }
                    }
                }
            }
        }
        
        // 關閉輸入檔
        fclose(fqY); fclose(fqCb); fclose(fqCr);
        if(is_lossless){
            fclose(feY); fclose(feCb); fclose(feCr);
        }

        // 6.寫出BMP檔
        FILE *fpOut = fopen(outBmpFile, "wb");  // wb=write binary
        if(!fpOut) { 
            perror("Output file error"); 
            return 1; 
        }

        // 歸0
        BITMAPFILEHEADER fileHeader = {0};
        BITMAPINFOHEADER infoHeader = {0};

        // 寫Header
        fileHeader.bfType = 0x4D42;
        fileHeader.bfSize = bfSize;
        fileHeader.bfOffBits = 54;

        infoHeader.biSize = 40;
        infoHeader.biWidth = width;
        infoHeader.biHeight = height;
        infoHeader.biPlanes = 1;
        infoHeader.biBitCount = 24;
        infoHeader.biSizeImage = biSizeImage;
        infoHeader.biXPelsPerMeter = xpels;
        infoHeader.biYPelsPerMeter = ypels;
        infoHeader.biClrUsed = clrUsed;
        infoHeader.biClrImportant = clrImp;

        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpOut);    // Header寫入檔案    
        fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpOut);

        unsigned char padBuf[3] = {0,0,0};
        for(int i=0; i<absHeight; i++){
            
            // 寫入有效像素RBG
            fwrite(imgBuffer + (i * width * 3), 1, width * 3, fpOut);
            
            // Padding補0
            if(padding > 0) {
                fwrite(padBuf, 1, padding, fpOut);
            }
        }

        fclose(fpOut);
        printf("Reconstructed image saved to: %s\n", outBmpFile);
    
        // 7. (1a) 計算 RGB SQNR
        // 有損且有原檔
        if(!is_lossless && refBmpFile != NULL) {
            FILE *fpRef = fopen(refBmpFile, "rb");

            if(!fpRef) {
                 printf("Cannot open reference image for SQNR.\n"); 
                }

            else {
                fseek(fpRef, 54, SEEK_SET); // 54跳過header
                unsigned char *refBuffer = (unsigned char*)malloc(width * absHeight * 3);   //要動態空間
                
                for(int i=0; i < absHeight; i++){
                    fread(refBuffer + (i * width * 3), 1, width * 3, fpRef);
                    if(padding > 0) {
                        fread(padBuf, 1, padding, fpRef);   // 讀取並丟掉
                    }
                }

                fclose(fpRef);

                double signal_power[3] = {0}, error_power[3] = {0}; // B, G, R
                
                for(int i=0; i < width * absHeight; i++) {
                    for(int c=0; c<3; c++) { 

                        //原始檔
                        double orig = refBuffer[i *3 + c];  //第i像素 第c顏色
                        
                        //還原檔
                        double recon = imgBuffer[i *3 + c];

                        double err = orig - recon;
                        signal_power[c] += orig * orig;
                        error_power[c] += err * err;
                    }
                }

                printf("RGB SQNR (dB):\n");
                
                const char* rgb_names[] = {"Blue", "Green", "Red"};
                
                // 印出順序 R, G, B
                for(int c=2; c>=0; c--) { 
                    double sqnr = (error_power[c] == 0) ? 
                    999.9 : 10 * log10(signal_power[c] / error_power[c]);
                    
                    printf("Channel %s: %.2f dB\n", rgb_names[c], sqnr);
                }
                
                free(refBuffer);
            }
        }
        
        free(imgBuffer);
    }

    else if (method == 2) {
        if (argc != 5) {
            printf("Usage: decoder 2 <output.bmp> <ascii/binary> <input_rle>\n");
            return 1;
        }

        char *outFile = argv[2];
        char *mode = argv[3];
        char *inFile = argv[4];
        int is_binary = (strcmp(mode, "binary") == 0);

        FILE *fpIn = is_binary ? fopen(inFile, "rb") : fopen(inFile, "r");
        if (!fpIn) { perror("Input error"); return 1; }

        int width, height;
        if (is_binary) {
            fread(&width, 4, 1, fpIn);
            fread(&height, 4, 1, fpIn);
        } else {
            fscanf(fpIn, "%d %d", &width, &height);
        }
        int absHeight = abs(height);

        unsigned char *imgBuffer = (unsigned char *)malloc(width * absHeight * 3);
        
        int prevDC[3] = {0, 0, 0};
        const int (*qTables[3])[8] = {std_lum_qt, std_chr_qt, std_chr_qt};
        
        // 暫存三個通道的 Spatial Domain (8x8 Block)
        double blk_all[3][8][8]; 
        int quantized_block[64];

        printf("Method 2 Decoding (%s)...\n", mode);

        for (int r = 0; r < absHeight; r += 8) {
            if(r % 80 == 0) printf("Decoding Row %d / %d\n", r, absHeight); // 進度條

            for (int c = 0; c < width; c += 8) {
                
                // 依序還原 Y, Cb, Cr
                for (int k = 0; k < 3; k++) {
                    
                    // 清空 Buffer
                    for(int z=0; z<64; z++) quantized_block[z] = 0;

                    // A. 讀取 DPCM DC
                    int diffDC = 0;
                    if (is_binary) {
                        short sDiff;
                        fread(&sDiff, 2, 1, fpIn);
                        diffDC = sDiff;
                    } else {
                        // ASCII: 跳過 ($m,$n, Ch) 讀 diff
                        char ch;
                        while(fscanf(fpIn, "%c", &ch) && ch != ')');
                        int skip, val;
                        fscanf(fpIn, "%d %d", &skip, &val); // skip=0, val=diff
                        diffDC = val;
                    }

                    // 還原 DC
                    prevDC[k] += diffDC;
                    quantized_block[0] = prevDC[k];

                    // B. RLE 解碼 (AC)
                    int index = 1;
                    while (index < 64) {
                        int skip, val;
                        if (is_binary) {
                            unsigned char s; short v;
                            fread(&s, 1, 1, fpIn); fread(&v, 2, 1, fpIn);
                            skip = s; val = v;
                            if (skip == 0 && val == 0) break; // EOB
                        } else {
                            // ASCII: 檢查是否換行
                            char check;
                            do { check = fgetc(fpIn); } while (check == ' ');
                            if (check == '\n' || check == '\r' || check == EOF) break;
                            ungetc(check, fpIn);
                            fscanf(fpIn, "%d %d", &skip, &val);
                        }
                        index += skip;
                        if (index >= 64) break;
                        quantized_block[index] = val;
                        index++;
                    }

                    // C. 反 ZigZag + 反量化
                    double dct[8][8];
                    for (int z = 0; z < 64; z++) {
                        int u = zigzag_order[z] / 8;
                        int v = zigzag_order[z] % 8;
                        dct[u][v] = (double)quantized_block[z] * qTables[k][u][v];
                    }

                    // D. IDCT (結果存入 blk_all[k])
                    perform_IDCT(dct, blk_all[k]); 
                } 

                // E. 轉 RGB 填入 imgBuffer
                for(int i=0; i<8; i++){
                    for(int j=0; j<8; j++){
                        if (r+i < absHeight && c+j < width) {
                            double Y  = blk_all[0][i][j] + 128.0;
                            double Cb = blk_all[1][i][j];
                            double Cr = blk_all[2][i][j];

                            double R = Y + 1.402 * Cr;
                            double G = Y - 0.344136 * Cb - 0.714136 * Cr;
                            double B = Y + 1.772 * Cb;

                            int idx = ((r+i) * width + (c+j)) * 3;
                            imgBuffer[idx]     = CLAMP(B);
                            imgBuffer[idx + 1] = CLAMP(G);
                            imgBuffer[idx + 2] = CLAMP(R);
                        }
                    }
                }
            }
        }

        // 寫出 BMP
        FILE *fpOut = fopen(outFile, "wb");
        if(!fpOut) { perror("Output error"); return 1; }

        BITMAPFILEHEADER fileHeader = {0};
        fileHeader.bfType = 0x4D42;
        fileHeader.bfOffBits = 54;
        fileHeader.bfSize = 54 + width * absHeight * 3;

        BITMAPINFOHEADER infoHeader = {0};
        infoHeader.biSize = 40;
        infoHeader.biWidth = width;
        infoHeader.biHeight = height;
        infoHeader.biPlanes = 1;
        infoHeader.biBitCount = 24;
        
        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpOut);
        fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpOut);

        unsigned char padBuf[3] = {0};
        int padding = (4 - (width * 3) % 4) % 4;
        for(int i=0; i<absHeight; i++){
            fwrite(imgBuffer + (i * width * 3), 1, width * 3, fpOut);
            if(padding > 0) fwrite(padBuf, 1, padding, fpOut);
        }
        
        fclose(fpOut); fclose(fpIn); free(imgBuffer);
        printf("Decoding Done: %s\n", outFile);
    }

    return 0;
}