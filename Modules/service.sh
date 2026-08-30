while [ -z "$(getprop sys.boot_completed)" ]; do
sleep 10
done

until [ -d "/sdcard/Android" ]; do
sleep 3
done

sleep 15

/data/adb/modules/Zetamin/Zetamin