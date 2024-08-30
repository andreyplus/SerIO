#!/bin/sh

# === SET ENVIRONMENT application ===============================
BASE_DIR=$(dirname "$(readlink -f "$0")")
export LD_LIBRARY_PATH="$BASE_DIR"/lib/:"$BASE_DIR":$LD_LIBRARY_PATH
export QML_IMPORT_PATH="$BASE_DIR"/qml/:$QML_IMPORT_PATH
export QML2_IMPORT_PATH="$BASE_DIR"/qml/:$QML2_IMPORT_PATH
export QT_PLUGIN_PATH="$BASE_DIR"/plugins/:$QT_PLUGIN_PATH
export QTWEBENGINEPROCESS_PATH="$BASE_DIR"/QtWebEngineProcess
export QTDIR="$BASE_DIR"
export QT_QPA_PLATFORM_PLUGIN_PATH="$BASE_DIR"/plugins/platforms:$QT_QPA_PLATFORM_PLUGIN_PATH
# === RUN application ===========================================
"$BASE_DIR/./SerIO" "$@"
