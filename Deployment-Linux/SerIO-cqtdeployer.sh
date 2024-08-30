#!/bin/sh

# === CQtDeployer ===========================================
echo "SerIO - CQtDeployer:"
cqtdeployer -bin ~/MyWorks/SerIO/build-SerIO-Desktop_Qt_5_15_2_GCC_64bit-Release/SerIO -qmake ~/Qt/5.15.2/gcc_64/bin/qmak -targetDir ~/SerIO -binOut ./

# === Addition folders and files ============================
echo "SerIO - addition folders and files:"
if [ -d $HOME/SerIO ]; then cp -a $HOME/MyWorks/SerIO/Deployment-Linux/SerIO-AddFoldersFiles/. $HOME/SerIO/; fi
