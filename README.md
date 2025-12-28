# 多媒體訊號處理期末專案 (Multimedia Signal Processing Final Project)

**學號:** 411172018  
**姓名:** 李希均

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

此階段將量化後的係數進行 DPCM 與 RLE 編碼，並分為 ASCII (2a) 與 Binary (2b) 兩種模式。

#### Method 2(a): ASCII 模式 (Debug Use)

**目標**：輸出人類可讀的 RLE 文字檔，用於除錯與邏輯驗證。

```
[ Encoder (Mode 2 - ascii) ]
      ⬇
[ ZigZag 掃描 ] ➜ [ DC: DPCM ] ➜ [ AC: RLE ]
      ⬇
[ 輸出: rle_code.txt (可視化文字檔) ]
      ⬇
[ Decoder (Mode 2 - ascii) ]
      ⬇
[ 解析文字檔 ] ➜ [ 反 RLE & 反 ZigZag ] ➜ [ 呼叫 Method 1 核心 ]
      ⬇
[ 輸出: QRes_M2_Ascii.bmp ]
(驗證: 應與 Method 1(a) 的 QResKimberly.bmp 一致)
```

#### Method 2(b):Binary 模式 (Final Product)

**目標**：輸出緊湊的二進位壓縮檔，計算最終壓縮率。

```
[ Encoder (Mode 2 - binary) ]
      ⬇
[ ZigZag 掃描 ] ➜ [ DC/AC 編碼 ] ➜ [ Bit/Byte Packing ]
      ⬇
[ 輸出: rle_code.bin (二進位壓縮檔) ]
      ⬇
[ Decoder (Mode 2 - binary) ]
      ⬇
[ 讀取 Binary ] ➜ [ 解碼還原係數 ] ➜ [ 呼叫 Method 1 核心 ]
      ⬇
[ 輸出: QRes_M2_Bin.bmp ]
(驗證: 應與 Method 1(a) 的 QResKimberly.bmp 一致)
```

## 2. 工作日誌 (Work Log)

**第一天12/25**：建立 .h 檔 + 確立檔案架構 

**第二天12/26**：查詢資料 + 建立 method 0 + debug method 0 + 嘗試建立 workflow

**第三天12/27**：建立 method 1 + debug method 1 + 繼續查詢資料 (透過1D-Separable DCT加速運算)

**第四天12/28**：建立 method 2 + debug method 1 + debug method 2 + 確立 workflow + 一大堆debug + 寫readme

由於12/26才考完期末考第8科，故較晚開始執行期末作業，會再好好檢討自己的時間規劃。

## 3. GitHub Artifacts

**ResKimberly_M0.bmp**	

Method 0	      格式轉換測試，應與原圖一致。

**QResKimberly.bmp**	

Method 1(a)	      基礎 JPEG 壓縮結果，會有失真。

**ResKimberly_M1.bmp**	

Method 1(b)	      加上誤差補償的結果，應與原圖一致。

**QRes_M2_Ascii.bmp**	

Method 2(a)	      從 ASCII RLE 檔解碼的圖片，應與 QResKimberly.bmp 一致。

**QRes_M2_Bin.bmp**	

Method 2(b)	      從 Binary 壓縮檔解碼的圖片，應與 QResKimberly.bmp 一致。

**rle_code.bin**	      

Method 2(b)	      最終產出的二進位壓縮檔，用於計算壓縮率。

## 4. 心得及感想

這門多媒體訊號處理的期末專案，不僅讓我複習了課堂上的理論，更讓我有機會親手實作一個完整的影像壓縮系統。以前在課本上看到 DCT 公式只覺得複雜，實際用 C 語言寫出來並看到圖片成功轉換時，才真正理解頻率域 (Frequency Domain) 的意義，特別是在實作 Method 1(b) 時，透過補上誤差值 eF 就能讓失真的圖片瞬間變回無損，這讓我對量化誤差有了非常直觀的認識。而在處理 12MP 的大圖時，原本暴力的 2D-DCT 跑得非常慢，後來我改寫成 Separable DCT (先對列做 1D-DCT，再對行做 1D-DCT)，運算速度提升了數倍，這讓我學到了在工程實務上「演算法複雜度」的重要性。再除錯上 Method 2(b) 是最困難的一關。將資料寫入 .bin 檔時，看不見內容只能靠 Hex Editor 檢查。我一度因為沒有處理好檔案結尾 (EOF) 讀取，導致解碼器無限迴圈。後來我先用 Method 2(a) 的 ASCII 模式把邏輯測通，再轉寫成 Binary 模式，這個策略幫了大忙。再來則是自動化測試，這是第一次我自己實際負責在作業中導入 GitHub Actions。雖然寫 .yml 檔還需要很多AI協助，也花了不少時間，但看到伺服器自動幫我跑完 Method 0 到 Method 2 的所有測試，並用 cmp 指令告訴我 "Images are identical"，那種安全感是手動測試無法比擬的。這讓我體會到了現代軟體工程中 DevOps 的價值。最後是想檢討自己在時辰規劃上，還是有很多不夠好的地方，像錯估 debug 的量和難度，導致整體一直 delay ，在本就不充裕的時間內，沒辦法快速有效率地完成 method 0-3 ，但這也讓我學習到，在稍微大一點的專案當中，預留更多一點 debug 時間的重要性，以往的小程式可能可以一次寫完，就可以在很清楚的狀況下去處理 bug ，但隨著程式規模變大，如何有效率的 debug ，就變得非常重要。
