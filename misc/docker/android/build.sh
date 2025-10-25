#!/bin/bash
set -e

echo "==> Building with Gradle..."

echo "APP_NAME: $SELENE_APP_NAME"
echo "APP_VERSION: $SELENE_APP_VERSION"
echo "APP_NAMESPACE: $SELENE_APP_NAMESPACE"

cmake -DSELENE_APP_NAME=$SELENE_APP_NAME -P /android/GenStringXml.cmake
# cat /android/app/src/main/res/values/strings.xml

gradle assemble
# cp -r /android /dist/android && \

echo "==> Copying Debug APK..."
cp /android/app/build/outputs/apk/debug/*.apk /dist/selene-0.5.0-Android-Debug.apk

echo "==> Copying Release APK..."
cp /android/app/build/outputs/apk/release/*.apk /dist/selene-0.5.0-Android.apk