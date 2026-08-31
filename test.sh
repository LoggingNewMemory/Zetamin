#!/system/bin/sh
# Zetamin Tester Script

OUTPUT="results.txt"

echo "====================================" > $OUTPUT
echo "        Zetamin Test Report         " >> $OUTPUT
echo "====================================" >> $OUTPUT
echo "" >> $OUTPUT

echo "[+] Device Information" >> $OUTPUT
echo "Brand: $(getprop ro.product.brand)" >> $OUTPUT
echo "Manufacturer: $(getprop ro.product.manufacturer)" >> $OUTPUT
echo "Model: $(getprop ro.product.model)" >> $OUTPUT
echo "Android Version: $(getprop ro.build.version.release)" >> $OUTPUT
echo "ROM/OS Info:" >> $OUTPUT
getprop | grep -iE "ro.miui.ui.version.name|ro.build.version.oplusrom|ro.vendor.build.fingerprint|ro.build.display.id" >> $OUTPUT
echo "" >> $OUTPUT

echo "[+] Refresh Rate Settings (SYSTEM)" >> $OUTPUT
settings list system | grep -iE "refresh|fps|rate|display" >> $OUTPUT
echo "" >> $OUTPUT

echo "[+] Refresh Rate Settings (GLOBAL)" >> $OUTPUT
settings list global | grep -iE "refresh|fps|rate|display" >> $OUTPUT
echo "" >> $OUTPUT

echo "[+] Refresh Rate Settings (SECURE)" >> $OUTPUT
settings list secure | grep -iE "refresh|fps|rate|display" >> $OUTPUT
echo "" >> $OUTPUT

echo "[+] SurfaceFlinger Status" >> $OUTPUT
dumpsys SurfaceFlinger | grep -iE "peak-refresh-rate|refresh-rate" | head -n 20 >> $OUTPUT
echo "" >> $OUTPUT

echo "[+] Display Modes & Limits" >> $OUTPUT
dumpsys display | grep -iE "mMinRefreshRate|mMaxRefreshRate|mDefaultPeakRefreshRate|supportedModes|mOverrideDisplayInfo" | head -n 15 >> $OUTPUT
echo "" >> $OUTPUT

echo "[+] Real FPS Detection Test" >> $OUTPUT
real_fps=$(cmd display dump 2>/dev/null | grep -Eo 'fps=[0-9.]+' | cut -f2 -d= | sort -nr | head -n1 | cut -d . -f 1)
if [ -z "$real_fps" ]; then
    real_fps=$(dumpsys display 2>/dev/null | grep -Eo 'fps=[0-9.]+' | cut -d= -f2 | sort -nr | head -n1 | cut -d . -f 1)
fi
echo "Detected Max FPS: $real_fps" >> $OUTPUT
echo "" >> $OUTPUT

echo "====================================" >> $OUTPUT
echo "            Test Complete           " >> $OUTPUT
echo "====================================" >> $OUTPUT

echo "Done! Send results.txt to the developer."
