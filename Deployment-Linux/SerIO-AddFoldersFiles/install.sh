#!/bin/sh

# === SETUP application =======================================
# === Creating or Replasce desktop file
echo -n "" > $HOME/SerIO/SerIO.desktop
# === Generation new data in the desktop file
echo "[Desktop Entry]" >> $HOME/SerIO/SerIO.desktop
echo "Type=Application" >> $HOME/SerIO/SerIO.desktop
echo "Name=SerIO" >> $HOME/SerIO/SerIO.desktop
echo "GenericName=Terminal" >> $HOME/SerIO/SerIO.desktop
echo "Comment=Special cross-platform terminal program" >> $HOME/SerIO/SerIO.desktop
echo "Categories=Utility; Development; Debugger; Electronics; HamRadio;" >> $HOME/SerIO/SerIO.desktop
echo "Path=$HOME/SerIO/" >> $HOME/SerIO/SerIO.desktop
echo "Icon=$HOME/SerIO/serio.png" >> $HOME/SerIO/SerIO.desktop
echo "Exec=sh $HOME/SerIO/SerIO.sh" >> $HOME/SerIO/SerIO.desktop
chmod +x  $HOME/SerIO/SerIO.desktop
# === Intergation desktop files in the system
if ! [ -d $HOME/.local/share/applications ]; then mkdir -p $HOME/.local/share/applications; fi
cp $HOME/SerIO/SerIO.desktop $HOME/.local/share/applications/
if [ -d $HOME/Desktop/ ]; then cp $HOME/SerIO/SerIO.desktop $HOME/Desktop; fi
if [ -d $HOME/Рабочий\ стол/ ]; then cp $HOME/SerIO/SerIO.desktop $HOME/Рабочий\ стол; fi
# === Addition application fonts
mkdir -p $HOME/.fonts
cp $HOME/SerIO/fonts/Ubuntu*.ttf $HOME/.fonts/
