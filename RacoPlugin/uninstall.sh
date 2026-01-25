# Use Shell (Do not use Magisk Module Syntax)
FILE="/data/ProjectRaco/raco.txt"

if grep -q -E "INCLUDE_ZETA=0|INCLUDE_AME=0" "$FILE"; then
    sed -i 's/INCLUDE_ZETA=0/INCLUDE_ZETA=1/g' "$FILE"
    sed -i 's/INCLUDE_AME=0/INCLUDE_AME=1/g' "$FILE"
    echo "Detected AmeRender or Vestia Zeta Enabled, This have been disabled."
fi

echo "Reboot after install =w="
