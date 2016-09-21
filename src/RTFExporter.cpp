#include "stdafx.h"
#include "Scintilla.h"
#include "ExportStructs.h"
#include "Exporter.h"
#include "RTFExporter.h"
#include <stdio.h>

RTFExporter::RTFExporter(void) {
	setClipboardID( RegisterClipboardFormat(CF_RTF));
	if (getClipboardID() == 0) {
		MessageBox(NULL, TEXT("Unable to register clipboard format RTF!"), TEXT("Error"), MB_OK);
	}
}

RTFExporter::~RTFExporter(void) {
}

bool RTFExporter::exportData(ExportData * ed) {
	//estimate buffer size needed
	char * buffer = ed->csd->dataBuffer;
	bool isUnicode = (ed->csd->currentCodePage == SC_CP_UTF8);

	int totalBytesNeeded = 1;	//zero terminator
	
	totalBytesNeeded += EXPORT_SIZE_RTF_STATIC + EXPORT_SIZE_RTF_STYLE * ed->csd->nrUsedStyles + ed->csd->totalFontStringLength + EXPORT_SIZE_RTF_SWITCH * ed->csd->nrStyleSwitches;

	unsigned char testChar = 0;
	for(int i = 0; i < ed->csd->nrChars; i++) {
		testChar = buffer[(i*2)];
		switch(testChar) {
			case '{':
				totalBytesNeeded += 2;	// '\{'
				break;
			case '}':
				totalBytesNeeded += 2;	// '\}'
				break;
			case '\\':
				totalBytesNeeded += 2;	// '\\'
				break;
			case '\t':
				totalBytesNeeded += 5;	// '\tab '
				break;
			case '\r':
				if (buffer[(i+1)*2] == '\n')
					break;
			case '\n':
				totalBytesNeeded += 6;	// '\par\r\n'
				break;
			default:
				if (testChar < 0x80 || !isUnicode)
					totalBytesNeeded += 1;	// 'char'
				else {
					totalBytesNeeded += 8;	// '\u#####?
					i++;
					if (testChar >= 0xE0)
						i++;
				}

				break;
		}
	}

	int currentBufferOffset = 0;
	HGLOBAL hRTFBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, totalBytesNeeded);
	char * clipbuffer = (char *)GlobalLock(hRTFBuffer);
	clipbuffer[0] = 0;

	int txSize = ed->csd->tabSize * ed->csd->twipsPerSpace;

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "{\\rtf1\\ansi\\deff0\\deftab%u\r\n\r\n", txSize);
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "{\\fonttbl\r\n");

	StyleData * currentStyle;

	int currentFontIndex = 0;
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "{\\f%03d %s;}\r\n", currentFontIndex, (ed->csd->styles+STYLE_DEFAULT)->fontString);
	currentFontIndex++;

	for(int i = 0; i < NRSTYLES; i++) {
		if (i == STYLE_DEFAULT)
			continue;
		if (ed->csd->usedStyles[i] == true) {
			currentStyle = (ed->csd->styles)+i;
			currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "{\\f%03d %s;}\r\n", currentFontIndex, currentStyle->fontString);
			if ( !strcmp(currentStyle->fontString, (ed->csd->styles+STYLE_DEFAULT)->fontString) ) {
				currentStyle->fontIndex = (ed->csd->styles+STYLE_DEFAULT)->fontIndex;
			} else {
				currentStyle->fontIndex = currentFontIndex;
			}
			currentFontIndex++;
		}
	}


	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "}\r\n\r\n");	//fonttbl
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "{\\colortbl\r\n");

	int currentColorIndex = 0;
	for(int i = 0; i < NRSTYLES; i++) {
		if (ed->csd->usedStyles[i] == true) {
			currentStyle = (ed->csd->styles)+i;

			currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\red%03d\\green%03d\\blue%03d;\r\n", (currentStyle->fgColor>>0)&0xFF, (currentStyle->fgColor>>8)&0xFF, (currentStyle->fgColor>>16)&0xFF);
			currentStyle->fgClrIndex = currentColorIndex;
			currentColorIndex++;

			currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\red%03d\\green%03d\\blue%03d;\r\n", (currentStyle->bgColor>>0)&0xFF, (currentStyle->bgColor>>8)&0xFF, (currentStyle->bgColor>>16)&0xFF);
			currentStyle->bgClrIndex = currentColorIndex;
			currentColorIndex++;
		}
	}

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "}\r\n\r\n");	//colortbl

//-------Dump text to RTF
	int lastStyle = -1;
	int prevStyle = STYLE_DEFAULT;
	int bufferStyle = STYLE_DEFAULT;
	unsigned char currentChar;
	StyleData * styles = ed->csd->styles;
	utf16 unicodeValue;

	//print default style information
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\f%d\\fs%d\\cb%d\\cf%d ", styles[STYLE_DEFAULT].fontIndex, styles[STYLE_DEFAULT].size * 2, styles[STYLE_DEFAULT].bgClrIndex, styles[STYLE_DEFAULT].fgClrIndex);

	for(int i = 0; i < ed->csd->nrChars; i++) {
		currentChar = buffer[(i*2)];
		bufferStyle = buffer[(i*2)+1];

		//print new style info if style changes
		if (lastStyle != bufferStyle) {
			if (lastStyle != -1)
				prevStyle = lastStyle;
			lastStyle = bufferStyle;
			//currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\f%d\\fs%d\\cb%d\\cf%d", styles[lastStyle].fontIndex, styles[lastStyle].size * 2, styles[lastStyle].bgClrIndex, styles[lastStyle].fgClrIndex);
			
			if (styles[lastStyle].fontIndex != styles[prevStyle].fontIndex) {
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\f%d", styles[lastStyle].fontIndex);
			}
			if (styles[lastStyle].size != styles[prevStyle].size) {
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\fs%d", styles[lastStyle].size * 2);
			}
			if (styles[lastStyle].bgClrIndex != styles[prevStyle].bgClrIndex) {
				//currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\cb%d", styles[lastStyle].bgClrIndex);
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\highlight%d", styles[lastStyle].bgClrIndex);
			}
			if (styles[lastStyle].fgClrIndex != styles[prevStyle].fgClrIndex) {
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\cf%d", styles[lastStyle].fgClrIndex);
			}
			///////////////////////
			if (styles[lastStyle].bold != styles[prevStyle].bold) {
				if (styles[lastStyle].bold) {
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\b");
				} else {
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\b0");
				}
			}
			if (styles[lastStyle].italic != styles[prevStyle].italic) {
				if (styles[lastStyle].underlined) {
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\i");
				} else {
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\i0");
				}
			}
			if (styles[lastStyle].underlined != styles[prevStyle].underlined) {
				if (styles[lastStyle].underlined) {
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\ul");
				} else {
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\ul0");
				}
			}
			currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, " ");
		}

		//print character, parse special ones
		switch(currentChar) {
			case '{':
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\{");
				break;
			case '}':
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\}");
				break;
			case '\\':
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\\\");
				break;
			case '\t':
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\tab ");
				break;
			case '\r':
				if (buffer[(i*2)+2] == '\n')
					break;
			case '\n':
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\par\r\n");
				break;
			default:
				if (currentChar < 0x20)	//ignore control characters
					break;
				if (currentChar > 0x7F && isUnicode) {	//this may be some UTF-8 character, so parse it as such
					unicodeValue.value = 0;

					if (currentChar < 0xE0) {
						unicodeValue.value  = ((0x1F & currentChar) << 6);
						i++; currentChar = buffer[(i*2)];
						unicodeValue.value |=  (0x3F & currentChar);
					} else {
						unicodeValue.value  = ((0xF & currentChar) << 12);
						i++; currentChar = buffer[(i*2)];
						unicodeValue.value |= ((0x3F & currentChar) << 6);
						i++; currentChar = buffer[(i*2)];
						unicodeValue.value |=  (0x3F & currentChar);
					}

					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\\u%d?", unicodeValue.value);	//signed values
				} else {
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "%c", currentChar);
				}
				break;
		}
	}

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "}\r\n");	//rtf/ansi

	//char text[500];
	//sprintf(text, "RTF: of %d allocated, %d was used", totalBytesNeeded, currentBufferOffset);
	//MessageBox(NULL, (text), NULL, MB_OK);

	GlobalUnlock(hRTFBuffer);
	ed->hBuffer = hRTFBuffer;
	ed->bufferSize = currentBufferOffset;
	return true;
}

TCHAR * RTFExporter::getClipboardType() {
	return CF_RTF;
}
