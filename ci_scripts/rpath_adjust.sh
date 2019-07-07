#!/bin/sh
#cd ${HOME}/build/${TRAVIS_REPO_SLUG}/build
echo $(pwd)
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/bearer/libqgenericbearer.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/iconengines/libqsvgicon.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqgif.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqicns.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqico.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqjpeg.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqmacheif.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqmacjp2.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqsvg.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqtga.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqtiff.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqwbmp.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/imageformats/libqwebp.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/platforminputcontexts/libqtvirtualkeyboardplugin.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/position/libqtposition_cl.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/position/libqtposition_positionpoll.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/position/libqtposition_serialnmea.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/printsupport/libcocoaprintersupport.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/styles/libqmacstyle.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/virtualkeyboard/libqtvirtualkeyboard_hangul.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/virtualkeyboard/libqtvirtualkeyboard_openwnn.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/virtualkeyboard/libqtvirtualkeyboard_pinyin.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/virtualkeyboard/libqtvirtualkeyboard_tcime.dylib
install_name_tool -add_rpath @loader_path/../../Frameworks ./bin/Sigil.app/Contents/PlugIns/virtualkeyboard/libqtvirtualkeyboard_thai.dylib
