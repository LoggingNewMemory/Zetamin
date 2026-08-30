while [ -z "$(getprop sys.boot_completed)" ]; do
sleep 10
done
/data/adb/modules/Zetamin/Zetamin