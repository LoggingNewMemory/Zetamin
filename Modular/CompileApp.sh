#!/bin/bash

echo "---------------------------------"
echo "       Building the App          "
echo "---------------------------------"

# Setup variables based on project
APP_DIR="AppSource"
KEYSTORE_NAME="AppKey.jks"
KEY_ALIAS="key0"
KS_PASS="aw240706"
MODULES_DIR="Modules"

if [ ! -d "$APP_DIR" ]; then
    echo "No AppSource directory found. Skipping app build."
    exit 0
fi

cd "$APP_DIR" || exit 1
JAVA_HOME=/usr/lib/jvm/java-17-openjdk ./gradlew assembleRelease
cd ..

APK_UNSIGNED="${APP_DIR}/app/build/outputs/apk/release/app-release-unsigned.apk"
APK_SIGNED="${APP_DIR}/app/build/outputs/apk/release/app-release.apk"
APK_ALIGNED="${APP_DIR}/app/build/outputs/apk/release/app-release-aligned.apk"

# Determine module id for output apk name
MODULE_ID=$(grep "^id=" "$MODULES_DIR/module.prop" | cut -d'=' -f2 | tr -d '[:space:]')
APK_OUTPUT_NAME="${MODULE_ID}.apk"

# If gradle didn't produce a signed APK but did produce an unsigned one, sign it
if [ ! -f "$APK_SIGNED" ] && [ -f "$APK_UNSIGNED" ]; then
    echo "Unsigned APK detected. Signing with ${KEYSTORE_NAME}..."
    
    # Find latest build-tools
    BUILD_TOOLS_DIR=$(ls -1d /home/yamada/Android/Sdk/build-tools/* 2>/dev/null | sort -V | tail -n 1)
    APKSIGNER="$BUILD_TOOLS_DIR/apksigner"
    ZIPALIGN="$BUILD_TOOLS_DIR/zipalign"
    
    if [ -f "$APKSIGNER" ] && [ -f "$ZIPALIGN" ]; then
        if [ ! -f "$KEYSTORE_NAME" ]; then
            echo "ERROR: Keystore $KEYSTORE_NAME not found!"
            exit 1
        fi

        echo "Aligning APK..."
        "$ZIPALIGN" -v -p 4 "$APK_UNSIGNED" "$APK_ALIGNED" > /dev/null
        
        echo "Signing APK..."
        "$APKSIGNER" sign --ks "$KEYSTORE_NAME" --ks-key-alias "$KEY_ALIAS" --ks-pass "pass:$KS_PASS" --key-pass "pass:$KS_PASS" --out "$APK_SIGNED" "$APK_ALIGNED"
        
        if [ $? -eq 0 ]; then
            echo "Signing successful!"
            rm -f "$APK_ALIGNED"
        else
            echo "ERROR: Signing failed!"
        fi
    else
        echo "WARNING: apksigner or zipalign not found in /home/yamada/Android/Sdk/build-tools/. Skipping signing."
    fi
fi

if [ -f "$APK_SIGNED" ]; then
    cp "$APK_SIGNED" "$MODULES_DIR/$APK_OUTPUT_NAME"
    echo "Copied app-release.apk to Modules/$APK_OUTPUT_NAME"
elif [ -f "$APK_UNSIGNED" ]; then
    cp "$APK_UNSIGNED" "$MODULES_DIR/$APK_OUTPUT_NAME"
    echo "Copied app-release-unsigned.apk to Modules/$APK_OUTPUT_NAME (Unsigned)"
else
    echo "ERROR: No release APK found!"
fi
