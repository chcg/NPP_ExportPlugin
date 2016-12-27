# NPP_Export
Notepad++ Plugin NppExport
unofficial repo with sourcecode of https://sourceforge.net/projects/npp-plugins/files/NppExport/


Build Status
------------

AppVeyor `VS2013` and `VS2015`  [![Build status](https://ci.appveyor.com/api/projects/status/hxhqdk46mjgh8d33?svg=true)](https://ci.appveyor.com/project/chcg/npp-exportplugin)


Description
------------

Plugin to export selection or file to clipboard or file in RTF or HTML.

Build
------------
Build with visual studio 2013 or higher

Changes
------------

0.2.8
* Add UNICODE caps.
* Export all formats now includes CF_TEXT
* Reverted to static menu name

0.2.7
* Fix missing characters above 127 (ie 'eating accented characters').
* Filters out indicator flags, should prevent out of bounds problems.
* HTML uses UTF-8 encoding tag when Scintilla is set to UTF-8.
* HTML uses \<pre\> tag, spaces and tab characters take one space.
* HTML uses sc## tag instead of StyleClass## (smaller files).
* RTF parses UTF8 data to enable unicode characters.
* RTF uses new system for tabstops (better accuracy).

0.2.6
* split exporter types in classes.
* Add option to clipboard RTF _and_ HTML.

0.2.5:
* Fix HTML clipboard.

0.2:
* Allow copy to clipboard.
* Fix some RTF stuff.

0.0.0.1:
* Initial release.


Notes
------------

Scintilla uses the default style to determine the tabstops (every tabsize spaces)
These tabstops are used for every style, so tabs may not match the use of spaces,
especially on non-proportional fonts (and using UTF8 makes things even worse, 
although it should have no effect on indents).
HTML makes it very hard to mimick this behaviour, so tabs may not always
align your text as they do in Scintilla. For best results just use the same font
for all styles, its usually the colors that make most of the difference anyway ;).