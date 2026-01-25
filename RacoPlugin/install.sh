#!/system/bin/sh
FILE="/data/ProjectRaco/raco.txt"

if grep -q -E "INCLUDE_ZETA=1|INCLUDE_AME=1" "$FILE"; then
    sed -i 's/INCLUDE_ZETA=1/INCLUDE_ZETA=0/g' "$FILE"
    sed -i 's/INCLUDE_AME=1/INCLUDE_AME=0/g' "$FILE"
    echo "Detected AmeRender or Vestia Zeta Enabled, This have been disabled."
fi

echo "Reboot after install =w="