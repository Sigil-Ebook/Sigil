# Needs to be run after building on Mac

echo "Copying the 'data' folder to app bundle..."
cp -R release/data Sigil.app/Contents/MacOS/

echo "Adding Qt frameworks and plugins to app bundle..."
macdeployqt Sigil.app -dmg

