#!/bin/sh

printUsage() {
  echo "$0 <sqlitestudio build output directory> <qmake path> [dmg|dist|dist_full]"
}

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
  printUsage
  exit 1
fi

if [ "$#" -eq 3 ] && [ "$3" != "dmg" ] && [ "$3" != "dist" ] && [ "$3" != "dist_plugins" ] && [ "$3" != "dist_full" ]; then
  printUsage
  exit 1
fi

qt_deploy_bin="${2/qmake/macdeployqt}"
ls $qt_deploy_bin >/dev/null 2>&1
if [ "$?" -ne 0 ]; then
  echo "macdeployqt program missing!"
  exit 1
fi

cd $1/SQLiteStudio

rm -rf SQLiteStudio.app/Contents/Frameworks
rm -rf SQLiteStudio.app/Contents/PlugIns
rm -f SQLiteStudio.app/Contents/MacOS/sqlitestudiocli
rm -f SQLiteStudio.app/Contents/Resources/qt.conf

mkdir SQLiteStudio.app/Contents/Frameworks

cp -RP plugins SQLiteStudio.app/Contents
mv SQLiteStudio.app/Contents/plugins SQLiteStudio.app/Contents/PlugIns

mkdir -p SQLiteStudio.app/Contents/PlugIns/styles
cp -RP styles/* SQLiteStudio.app/Contents/PlugIns/styles

cp -RP lib*SQLiteStudio*.dylib SQLiteStudio.app/Contents/Frameworks

# CLI paths
qtcore_path=`otool -L sqlitestudiocli | grep QtCore | awk '{print $1;}'`
new_qtcore_path="@rpath/QtCore.framework/Versions/5/QtCore"

cp -P sqlitestudiocli SQLiteStudio.app/Contents/MacOS
install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/sqlitestudiocli
install_name_tool -change $qtcore_path $new_qtcore_path SQLiteStudio.app/Contents/MacOS/sqlitestudiocli

# SQLiteStudio binary paths
install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/SQLiteStudio
install_name_tool -change libguiSQLiteStudio.1.dylib "@rpath/libguiSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/SQLiteStudio

# Lib paths
install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/Frameworks/libguiSQLiteStudio.1.dylib
install_name_tool -change libsqlite3.0.dylib "@rpath/libsqlite3.0.dylib" SQLiteStudio.app/Contents/Frameworks/libcoreSQLiteStudio.1.dylib

echo "lib:"
ls -l ../../../lib/

echo "in frameworks - 1:"
ls -l SQLiteStudio.app/Contents/Frameworks

cp -RP ../../../lib/*.dylib SQLiteStudio.app/Contents/Frameworks

echo "in frameworks - 2:"
ls -l SQLiteStudio.app/Contents/Frameworks

# Plugin paths
function fixPluginPaths() {
    for f in `ls $1`
    do
        PLUGIN_FILE=$1/$f
        if [ -f $PLUGIN_FILE ]; then
    	    echo "Fixing paths for plugin $PLUGIN_FILE"
            install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" $PLUGIN_FILE
            install_name_tool -change libguiSQLiteStudio.1.dylib "@rpath/libguiSQLiteStudio.1.dylib" $PLUGIN_FILE
        fi
        if [ -d $PLUGIN_FILE ]; then
            fixPluginPaths $PLUGIN_FILE
        fi
    done
}
fixPluginPaths SQLiteStudio.app/Contents/PlugIns

function replaceInfo() {
	cdir=`pwd`
    echo Replacing Info.plist
    cd $1/SQLiteStudio
    VERSION=`SQLiteStudio.app/Contents/MacOS/sqlitestudiocli -v | awk '{print $2}'`
    YEAR=`date '+%Y'`

    cd SQLiteStudio.app/Contents
    sed "s/%VERSION%/$VERSION/g" Info.plist | sed "s/%YEAR%/$YEAR/g" > Info.plist.new
    echo "New plist:"
    cat Info.plist.new
    mv Info.plist.new Info.plist
	cd $cdir
}


if [ "$3" == "dmg" ]; then
    replaceInfo $1
    $qt_deploy_bin SQLiteStudio.app -dmg
elif [ "$3" == "dist" ]; then
	replaceInfo $1
	echo "in frameworks - 3:"
	ls -l SQLiteStudio.app/Contents/Frameworks
	$qt_deploy_bin SQLiteStudio.app -dmg -executable=SQLiteStudio.app/Contents/MacOS/SQLiteStudio -always-overwrite -verbose=3

	cd $1/SQLiteStudio
	VERSION=`SQLiteStudio.app/Contents/MacOS/sqlitestudiocli -v | awk '{print $2}'`

	mv SQLiteStudio.dmg sqlitestudio-$VERSION.dmg
	
	hdiutil attach sqlitestudio-$VERSION.dmg
	cd /Volumes/SQLiteStudio
	echo "in frameworks - 4:"
	ls -l SQLiteStudio.app/Contents/Frameworks
	
    echo "Done."
else
    replaceInfo $1
    $qt_deploy_bin SQLiteStudio.app
fi
