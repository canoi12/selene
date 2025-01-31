#/usr/bin/bash

OUTDIR=dist
TMPDIR=dist_tmp
BUILDSDIR=builds/
APPIMAGETOOL="./appimagetool-x86_64.AppImage"

if [ ! -d $BUILDSDIR ]; then
    ./scripts/build.sh
fi

if [ ! -d $OUTDIR ]; then
    mkdir -p $OUTDIR
fi

generate_appimage() {
    rm -r $TMPDIR/
    mkdir $TMPDIR/
    cp -r cross/AppImage $TMPDIR/AppImage

    mkdir -p $TMPDIR/AppImage/usr/bin
    mkdir -p $TMPDIR/AppImage/usr/lib

    cp $BUILDSDIR/$1/bin/selene $TMPDIR/AppImage/usr/bin/selene
    cp $BUILDSDIR/$1/lib/liblua5.4.so $TMPDIR/AppImage/usr/lib/liblua5.4.so
    cp $BUILDSDIR/$1/lib/libselene.so $TMPDIR/AppImage/usr/lib/libselene.so

    ARCH=""
    case $1 in
    "aarch64"* ) ARCH="aarch64" ;;
    "i686"* ) ARCH="i686" ;;
    "x86_64"* ) ARCH="x86_64" ;;
    * ) echo "Invalid architecture" ;;
    esac

    ARCH=$ARCH $APPIMAGETOOL "$TMPDIR/AppImage" "$OUTDIR/Selene-$ARCH.AppImage"

    rm -r $TMPDIR
}

generate_apk() {
    rm -r $TMPDIR/
    mkdir $TMPDIR/

    cp -r cross/android/template/* $TMPDIR/
    cd $TMPDIR
    ./gradlew build

    cd ..

    cp $TMPDIR/app/build/outputs/apk/release/*.apk "$OUTDIR/com.selene.app.apk"
}

aarch64_linux() {
    generate_appimage "aarch64-linux-gnu"
}

powerpc64_linux() {
    generate_appimage "powerpc64-linux-gnu"
}

i686_linux() {
    generate_appimage "i686-linux-gnu"
}

x86_64_linux() {
    generate_appimage "x86_64-linux-gnu"
}

generate_dist() {
    case $1 in
    # Linux
    'i686_linux' ) i686_linux ;;
    'x86_64_linux' ) x86_64_linux ;;
    'aarch64_linux' ) aarch64_linux ;;
    'powerpc64_linux' ) powerpc64_linux ;;
    # Android
    'android' ) generate_apk ;;
    'all' )
        # Linux
        i686_linux
        x86_64_linux
        aarch64_linux
        powerpc64_linux
        # Android
        generate_apk ;;
    esac
}

generate_dist $1