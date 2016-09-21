#pragma once

#define SCINTILLA_FONTNAME_SIZE	(32+1)
#define NRSTYLES	(STYLE_MAX+1)
struct StyleData {
	char fontString[32];
	int fontIndex;
	int size;
	int bold;
	int italic;
	int underlined;
	int fgColor;
	int bgColor;
	int fgClrIndex;
	int bgClrIndex;
	bool eolExtend;
};

struct CurrentScintillaData {
	HWND hScintilla;
	long nrChars;
	int tabSize;
	bool usedStyles[NRSTYLES];
	StyleData * styles;
	char * dataBuffer;
	int nrUsedStyles;
	int nrStyleSwitches;
	int totalFontStringLength;
	int currentCodePage;
	int twipsPerSpace;
};

struct ExportData {
	bool isClipboard;
	CurrentScintillaData * csd;
	HGLOBAL hBuffer;
	unsigned long bufferSize;
};
