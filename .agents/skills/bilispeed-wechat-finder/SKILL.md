---
name: bilispeed-wechat-finder
description: BiliSpeed 微信视频号默认倍速复测与适配维护流程。Use when working in the biliSpeed repo on WeChat/微信视频号/default playback speed/LSPosed/Vector/FinderHome/FinderThumbPlayerProxy regressions, updating VideoSpeed APK on rooted Android devices, or rechecking after WeChat version changes.
---

# BiliSpeed WeChat Finder

Use this project skill to avoid re-discovering the WeChat Video Channels path from scratch. The stable route for WeChat `8.0.69` is LSPosed Java hook into Finder feed, not ExoPlayer and not native FFmpeg/ThumbPlayer symbols.

## Known Good Path

- Default speed is applied by actively injecting on the current feed player after playback/navigation events.
- Runtime player class: `com.tencent.mm.plugin.finder.video.FinderThumbPlayerProxy`.
- Feed entry activity: `com.tencent.mm.plugin.finder.ui.FinderHomeAffinityUI`.
- Current holder path: `FinderHome` Activity -> `R.id.m6e` (`RefreshLoadMoreLayout`) -> `getRecyclerView()` -> current `me5.s0` holder -> `holder.o(R.id.e_k)` -> `FinderVideoLayout.getVideoView()`.
- Setter proven on WeChat `8.0.69` / `versionCode 3040`: `setPlaySpeed(float)`.
- Injection triggers proven useful: `Activity.onResume` delayed and `Activity.dispatchTouchEvent(ACTION_UP)` delayed.
- Settings page must make the app data directory executable, `shared_prefs` readable/executable, and `speed.xml` readable for `XSharedPreferences`. Do not make the app data root world-readable, or Android `run-as` will reject the package.

Do not start with native hooks, `libxffmpeg` filters, exported ThumbPlayer symbols, or ExoPlayer unless the Java feed path is proven dead on the current WeChat build.

## Fast Verification

Always use an explicit device serial when more than one Android device may be connected:

```powershell
$adb = 'F:\MarsDesktop\#Miui\#ADB\ADB\adb.exe'
$serial = '<device-serial>'
& $adb -s $serial devices -l
& $adb -s $serial shell getprop ro.product.device
& $adb -s $serial shell getprop ro.build.display.id
& $adb -s $serial shell getprop ro.boot.slot_suffix
& $adb -s $serial shell getprop sys.boot_completed
& $adb -s $serial shell dumpsys package io.github.MarsGao.speed | Select-String -Pattern 'versionName=|versionCode=|lastUpdateTime='
& $adb -s $serial shell dumpsys package com.tencent.mm | Select-String -Pattern 'versionName=|versionCode=|lastUpdateTime='
```

Install the current debug APK only after a successful build:

```powershell
$env:JAVA_HOME = 'C:\Program Files\Eclipse Adoptium\jdk-17.0.19.10-hotspot'
$env:ANDROID_HOME = 'C:\Users\k.gao\AppData\Local\Android\Sdk'
$env:ANDROID_SDK_ROOT = $env:ANDROID_HOME
.\gradlew.bat :app:assembleDebug
& $adb -s $serial install -r -d 'app\build\outputs\apk\debug\app-debug.apk'
```

Enter WeChat Video Channels:

```powershell
& $adb -s $serial shell am force-stop com.tencent.mm
Start-Sleep -Seconds 1
& $adb -s $serial shell monkey -p com.tencent.mm -c android.intent.category.LAUNCHER 1
Start-Sleep -Seconds 5
& $adb -s $serial shell input tap 900 3105
Start-Sleep -Seconds 1
& $adb -s $serial shell input tap 300 675
Start-Sleep -Seconds 5
& $adb -s $serial shell dumpsys window | Select-String -Pattern 'mCurrentFocus|mFocusedApp'
```

Success log patterns:

```text
[VideoSpeed] [FinderInject] Activity resume/touch hooks installed
[VideoSpeed] [FinderInject] q40.onClick hook installed
[VideoSpeed] FinderActivity.onResume current com.tencent.mm.plugin.finder.video.FinderThumbPlayerProxy setRate target via setPlaySpeed: 2.0
[VideoSpeed] FinderActivity.touchUp current com.tencent.mm.plugin.finder.video.FinderThumbPlayerProxy setRate target via setPlaySpeed: 1.5
```

Pull only the relevant tail:

```powershell
$out = & $adb -s $serial shell su -c 'cat /data/adb/lspd/log/modules_*.log /data/adb/lspd/log/verbose_*.log 2>/dev/null | tail -n 5000'
$out | Select-String -Pattern '\[VideoSpeed\].*(FinderInject|setRate target|manual speed)' | Select-Object -Last 60
```

If `su -c` is unavailable on KernelSU-style devices, use visible behavior plus LSPosed/Vector manager logs if available; do not treat missing `su` alone as module failure.

## Settings Validation

Use both UI behavior and runtime logs. The WeChat UI may not show `2x` or `1.5x` even when playback speed is injected.

When checking a changed default speed:

1. Open BiliSpeed.
2. Set speed to `2.0` or `1.5`.
3. Tap `设置`.
4. Force-stop WeChat.
5. Re-enter Video Channels.
6. Confirm the latest log says `setPlaySpeed: <target>`.

If the settings page appears to change but logs keep using the old target, inspect:

```powershell
& $adb -s $serial shell run-as io.github.MarsGao.speed cat shared_prefs/speed.xml 2>$null
& $adb -s $serial shell su -c 'ls -ld /data/data/io.github.MarsGao.speed /data/data/io.github.MarsGao.speed/shared_prefs; cat /data/data/io.github.MarsGao.speed/shared_prefs/speed.xml 2>/dev/null'
```

Known fix in `1.2.3`: commit preferences directly with `putFloat(...).commit()`, then make the app data directory executable, `shared_prefs` executable, and `speed.xml` world-readable.

If logs still keep the default target after `speed.xml` is correct, suspect `XSharedPreferences` file visibility. On Ace 5 / `PKG110_16.0.0.205(CN01)`, `1.2.3` entered `FinderHomeAffinityUI` and injected successfully, but WeChat still read fallback `2.0` after BiliSpeed was set to `1.5` until the app data directory, `shared_prefs`, and `speed.xml` permissions were corrected.

```powershell
& $adb -s $serial shell am start -n io.github.MarsGao.speed/.MainActivity
& $adb -s $serial shell run-as io.github.MarsGao.speed chmod 711 .
& $adb -s $serial shell run-as io.github.MarsGao.speed chmod 755 shared_prefs
& $adb -s $serial shell run-as io.github.MarsGao.speed chmod 644 shared_prefs/speed.xml
```

Known fix in `1.2.4`: `MainActivity.makePrefsReadable()` keeps the app data root at `711`, then makes `shared_prefs` and `speed.xml` readable before WeChat reads `XSharedPreferences`.

## Regression Triage

Use this order:

1. Verify package versions, LSPosed scope includes `com.tencent.mm`, and WeChat process restarts after install.
2. Verify user-visible playback and `FinderInject` logs.
3. If no `FinderInject` logs appear, verify `handleLoadPackage` is running for `com.tencent.mm`.
4. If `FinderInject` logs appear but no `setPlaySpeed` line appears, inspect whether `FinderHomeAffinityUI`, `R.id.m6e`, holder class `me5.s0`, or `R.id.e_k` changed.
5. If the setter fails, broad-probe only Java float speed setters in Finder/video/player/thumbplayer classes.
6. Only after Java path is proven absent, record the WeChat build as unsupported or start a new native investigation.

Avoid noisy global hooks in production. Broad `View.performClick`, all-class speed probes, and native Zygisk experiments are diagnosis-only and should be removed or disabled before release.

## Release Checklist

- Update `gradle.properties` `appVersionName`.
- Build with JDK 17 and confirm Gradle prints the expected versionCode.
- Install on at least one rooted test device and verify WeChat Video Channels behavior.
- Commit only source/docs/workflow changes; leave `zygisk-native/` and temporary logs out unless explicitly requested.
- Tag format for LSPosed: `VersionCode-VersionName`, for example `1002003-1.2.3`.
- Push `master` and tag to `origin`, wait for personal repo Actions release, then push `master` and tag to `official` and create the official GitHub release with the same APK asset.
