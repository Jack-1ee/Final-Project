# 多媒體訊號處理期末專題 (Multimedia Signal Processing Final Project)

**學號:** (請在此填寫您的學號)  
**姓名:** (請在此填寫您的姓名)

---

## 1. 系統實作架構與流程 (System Implementation & Workflow)

本專案依據 GitHub Actions 自動化測試流程 (Workflow)，完整實作了以下三個階段的影像處理方法。

### Method 0: 影像格式驗證 (Format Verification)
**目標**：確保 BMP 讀寫與記憶體操作正確，並通過 `cmp` 指令驗證無損還原。

```
[ Encoder (Mode 0) ]
      ⬇
[ 輸入: Kimberly.bmp ] ➜ [ 解析 Header & RGB ]
      ⬇
[ 輸出: 純文字檔 R.txt, G.txt, B.txt, dim.txt ]
      ⬇
[ Decoder (Mode 0) ]
      ⬇
[ 讀取文字檔 ] ➜ [ 重建 BMP Header & 像素 ]
      ⬇
[ 輸出: ResKimberly_M0.bmp ] 
(驗證: 與原圖 Kimberly.bmp 完全一致)
```

### Method 1: JPEG 基礎壓縮與誤差補償 (Baseline & Lossless)

此階段分為兩部分：1(a) 失真壓縮 與 1(b) 誤差補償無損還原。

#### Method 1(a): 基礎 JPEG (Baseline)

**目標**：實作 RGB $\leftrightarrow$ YCbCr、分離性 DCT、量化，產出失真還原圖。

```
[ Encoder (Mode 1) ]
      ⬇
[ YCbCr 轉換 ] ➜ [ 8x8 Separable DCT ] ➜ [ 量化 (Quantization) ]
      ⬇
[ 輸出: 量化係數 (qF_*.raw) & 量化表 (Qt_*.txt) ]
      ⬇
[ Decoder (Mode 1a) ]
      ⬇
[ 讀取 qF_*.raw ] ➜ [ 反量化 & IDCT ] ➜ [ YCbCr 轉 RGB ]
      ⬇
[ 輸出: QResKimberly.bmp ] 
(驗證: 觀察 SQNR 與壓縮失真效果)
```

#### Method 1(b): 誤差補償 (Error Residual)

**目標**：計算量化誤差 eF = F - qF * Qt，實現無損壓縮驗證。

```
[ Encoder (Mode 1) ]
      ⬇
[ 計算量化誤差 eF ] ➜ [ 輸出: 誤差檔 (eF_*.raw) ]
      ⬇
[ Decoder (Mode 1b) ]
      ⬇
[ 讀取 qF_*.raw + eF_*.raw ] ➜ [ 重組頻率域數值 (F = qF*Qt + eF) ]
      ⬇
[ 反轉換 IDCT & 後處理 ]
      ⬇
[ 輸出: ResKimberly_M1.bmp ]
(驗證: 加上誤差值後，應與原圖 Kimberly.bmp 完全一致)
```

### Method 2: 熵編碼 (Entropy Coding)
