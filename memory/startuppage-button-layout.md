---
name: startuppage-button-layout
description: StartPage (锁屏) Widget 层级、按钮关联和交互流程
metadata:
  type: project
---

## StartPage 当前布局（Designer 重新生成后）

### Widget 层级（添加顺序 = Z序由底到顶）

```
View (startuppageViewBase)
├── __background       (0,0 320x240) 黑色
├── flexButton1        (0,0 320x240) 全屏唤醒按钮 — 点击→function5()
├── blackCover         (0,0 320x240) 黑色遮罩 — 息屏用
├── scalableImage1     (0,0 320x240) 锁屏UI底图 BITMAP_LOCKED2_ID
├── modalWindow1       (全屏)        NFC提示弹窗 (初始hide)
│   ├── textArea1      (112, 109)    "请刷NFC" (可见)
│   └── textArea2      (103, 103)    "识别成功" (隐藏→NFC识别后显示)
└── flexButton2        (115,150,90,90) 透明NFC按钮 alpha=0
```

### 回调关系

| 控件 | 回调 | 触发 |
|------|------|------|
| flexButton1 (全屏) | flexButtonCallback | → function5() 唤醒解锁 |
| flexButton2 (透明) | nfcButtonCallback | → modalWindow1.show() 弹出NFC弹窗 |

### 交互相应

```
息屏态: blackCover盖在scalableImage1上 → 黑屏
       flexButton1全屏捕捉点击

唤醒: 点击flexButton1 → function5()
      → blackCover隐藏、flexButton1隐藏、flexButton2显示

点击NFC: 点击flexButton2(透明热区) → modalWindow1.show()
      → 显示textArea1 "请刷NFC"

刷卡: NFC识别 → updateNfcStatus()
      → textArea1隐藏、textArea2显示"识别成功"
      → 1秒后自动跳转HomePage

息屏: goToSleep()
      → blackCover显示、flexButton1显示、flexButton2隐藏
```
