#!/usr/bin/env bash

# A quick-and-dirty script to generate an AppImage
#
# Note: This script was meant to be use automatically by the Makefile
# Use it manully only for debugging purpose

SCRIPT=`basename $0`

function usage {
    echo "usage: "$SCRIPT" APP_NAME"
    exit -1
}

if [ $# -ne 1 ]
then
    usage
fi

MyApp=$1
myapp=`echo -n $1 | tr [:upper:] [:lower:]`
APP_DIR=$MyApp.AppDir
APPIMAGETOOL=appimagetool-x86_64.AppImage

DESKTOP=$APP_DIR/$myapp.desktop

rm -rf $APP_DIR/
mkdir -p $APP_DIR/usr/bin/ && \
echo "WARNING: no executable file provided in '"$MyApp".AppDir/usr/bin/'"

mkdir -p $APP_DIR/usr/lib/

#touch $APP_DIR/AppRun && chmod a+x $APP_DIR/AppRun && \
cp -v $MyApp $APP_DIR/AppRun
#echo "WARNING: empty AppRun file. You must provide it properly"

touch $APP_DIR/$MyApp.png # This PNG does not exist but it's ok.
#echo -e "WARNING: empty/invalid '"$myapp".png'. You must specifiy an image file"
#echo -e "INFO: You can provide PNG/JPEG/ICO file, but a PNG file is recommended\n"

echo "[Desktop Entry]" >> $DESKTOP
echo "Name="$MyApp >> $DESKTOP
echo "Exec="$MyApp >> $DESKTOP
echo "Icon=$myapp" >> $DESKTOP
echo "Type=Application" >> $DESKTOP
echo "Categories=Utility;" >> $DESKTOP
echo "Comment=" >> $DESKTOP
echo "Terminal=true" >> $DESKTOP

tree $MyApp.AppDir/
$APPIMAGETOOL $MyApp.AppDir/ $MyApp-x86_64.AppImage
