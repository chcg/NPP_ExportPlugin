#include "stdafx.h"
#include "Scintilla.h"
#include "ExportStructs.h"
#include "Exporter.h"
#include "HTMLExporter.h"
#include <stdio.h>

HTMLExporter::HTMLExporter(void) {
	setClipboardID(RegisterClipboardFormat(CF_HTML));
	if (getClipboardID() == 0) {
		MessageBox(NULL, TEXT("Unable to register clipboard format HTML!"), TEXT("Error"), MB_OK);
	}
}

HTMLExporter::~HTMLExporter(void) {
}

bool HTMLExporter::exportData(ExportData * ed) {

	//estimate buffer size needed
	char * buffer = ed->csd->dataBuffer;
	size_t totalBytesNeeded = 1;	//zero terminator
	bool addHeader = ed->isClipboard;	//true if putting data on clipboard
	
	totalBytesNeeded += EXPORT_SIZE_HTML_STATIC + EXPORT_SIZE_HTML_STYLE * (ed->csd->nrUsedStyles-1) + ed->csd->totalFontStringLength + EXPORT_SIZE_HTML_SWITCH * ed->csd->nrStyleSwitches;
	if (addHeader)
		totalBytesNeeded += EXPORT_SIZE_HTML_CLIPBOARD;
	if (ed->csd->currentCodePage == SC_CP_UTF8)
		totalBytesNeeded += EXPORT_SIZE_HTML_UTF8;
	int startHTML = 105, endHTML = 0, startFragment = 0, endFragment = 0;

	unsigned char testChar = 0;
	for(int i = 0; i < ed->csd->nrChars; i++) {
		testChar = buffer[(i*2)];
		switch(testChar) {
			case '\r':
				if (buffer[(i*2)+2] == '\n')
					break;
			case '\n':
				totalBytesNeeded += 2;	//	plain newline
				break;
			case '<':
				totalBytesNeeded += 4;	// '&lt;'
				break;
			case '>':
				totalBytesNeeded += 4;	// '&gt;'
				break;
			case '&':
				totalBytesNeeded += 5;	// '&amp;'
				break;
			case '\t':
				totalBytesNeeded += ed->csd->tabSize;
				break;
			default:
				if (testChar < 0x20)	//	ignore control characters
					break;
				totalBytesNeeded += 1; //	'char'
				break;
		}
	}

	int currentBufferOffset = 0;
	HGLOBAL hHTMLBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, totalBytesNeeded);
	if (hHTMLBuffer == nullptr)
	{
		return false;
	}

	char* clipbuffer = (char*)GlobalLock(hHTMLBuffer);
	if (clipbuffer == nullptr)
	{
		GlobalFree(hHTMLBuffer);
		return false;
	}

	clipbuffer[0] = 0;

	//add CF_HTML header if needed, return later to fill in the blanks
	if (addHeader) {
		currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "Version:0.9\r\nStartHTML:0000000105\r\nEndHTML:0000000201\r\nStartFragment:0000000156\r\nEndFragment:0000000165\r\n");
	}
	//end CF_HTML header

	//begin building context
	//proper doctype to pass validation, just because it looks good
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, 
	"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/1999/REC-html401-19991224/strict.dtd\">"
	"\r\n");
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<html>\r\n");

	//HACK!, this shouldn't belong here at <head>, but some programs refuse to read the CSS data provided, resulting in ugly text
	//Even still it sometimes gets ignored
	//add StartFragment if doing CF_HTML
	if (addHeader) {
		currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<!--StartFragment-->\r\n");
	}
	startFragment = currentBufferOffset;
	//end StartFragment

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<head>\r\n");

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<META http-equiv=Content-Type content=\"text/html; charset=");
	if (ed->csd->currentCodePage == SC_CP_UTF8) {
		currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "UTF-8");
	} else {
		//currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "iso-8859-1");
		currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "windows-1252");
	}
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\">\r\n");
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<title>Exported from Notepad++</title>\r\n");

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<style type=\"text/css\">\r\n");

	StyleData * currentStyle, * defaultStyle;
	defaultStyle = (ed->csd->styles)+STYLE_DEFAULT;

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "span {\r\n");
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-family: '%s';\r\n", defaultStyle->fontString);
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-size: %0dpt;\r\n", defaultStyle->size);
	if (defaultStyle->bold)		currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-weight: bold;\r\n");
	if (defaultStyle->italic)	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-style: italic;\r\n");
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tcolor: #%02X%02X%02X;\r\n", (defaultStyle->fgColor>>0)&0xFF, (defaultStyle->fgColor>>8)&0xFF, (defaultStyle->fgColor>>16)&0xFF);
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "}\r\n");

	for(int i = 0; i < NRSTYLES; i++) {
		if (i == STYLE_DEFAULT)
			continue;

		currentStyle = (ed->csd->styles)+i;
		if (ed->csd->usedStyles[i] == true) {
			currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, ".sc%d {\r\n", i);
			if (lstrcmpiA(currentStyle->fontString, defaultStyle->fontString))	//this is forcefully set to ANSI, this part of the plugin does not need Unicode
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-family: '%s';\r\n", currentStyle->fontString);
			if (currentStyle->size != defaultStyle->size)
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-size: %0dpt;\r\n", currentStyle->size);
			if (currentStyle->bold != defaultStyle->bold) {
				if (currentStyle->bold)
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-weight: bold;\r\n");
				else
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-weight: normal;\r\n");
			}
			if (currentStyle->italic != defaultStyle->italic) {
				if (currentStyle->italic)
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-style: italic;\r\n");
				else
					currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tfont-style: normal;\r\n");
			}
			if (currentStyle->underlined)
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\ttext-decoration: underline;\r\n");
			if (currentStyle->fgColor != defaultStyle->fgColor)
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tcolor: #%02X%02X%02X;\r\n", (currentStyle->fgColor>>0)&0xFF, (currentStyle->fgColor>>8)&0xFF, (currentStyle->fgColor>>16)&0xFF);
			if (currentStyle->bgColor != defaultStyle->bgColor)
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\tbackground: #%02X%02X%02X;\r\n", (currentStyle->bgColor>>0)&0xFF, (currentStyle->bgColor>>8)&0xFF, (currentStyle->bgColor>>16)&0xFF);
			currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "}\r\n");
		}
	}

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "</style>\r\n</head>\r\n");
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<body>\r\n");

	//end building context
/*
	//add StartFragment if doing CF_HTML
	if (addHeader) {
		currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<!--StartFragment-->\r\n");
	}
	startFragment = currentBufferOffset;
	//end StartFragment
*/
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<div style=\"");

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, 
		"float: left; "
		"white-space: pre; "
		"line-height: 1; "
		"background: #%02X%02X%02X; ",
		(defaultStyle->bgColor>>0)&0xFF, (defaultStyle->bgColor>>8)&0xFF, (defaultStyle->bgColor>>16)&0xFF
		);

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\">");

//-------Dump text to HTML
	char * tabBuffer = new char[static_cast<size_t>(ed->csd->tabSize) + 1];
	tabBuffer[0] = 0;
	for(int i = 0; i < ed->csd->tabSize; i++) {
		strcat(tabBuffer, " ");
	}

	int nrCharsSinceLinebreak = -1, nrTabCharsToSkip = 0;
	int lastStyle = -1;
	unsigned char currentChar;
	bool openSpan = false;

	for(int i = 0; i < ed->csd->nrChars; i++) {
		//print new span object if style changes
		if (buffer[i*2+1] != lastStyle) {
			if (openSpan) {
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "</span>");
			}
			lastStyle = buffer[i*2+1];
			currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<span class=\"sc%d\">", lastStyle);
			openSpan = true;
		}

		//print character, parse special ones
		currentChar = buffer[(i*2)];
		nrCharsSinceLinebreak++;
		switch(currentChar) {
			case '\r':
				if (buffer[(i*2)+2] == '\n')
					break;
			case '\n':
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "\r\n");
				nrCharsSinceLinebreak = -1;
				break;
			case '<':
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "&lt;");
				break;
			case '>':
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "&gt;");
				break;
			case '&':
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "&amp;");
				break;
			case '\t':
				nrTabCharsToSkip = nrCharsSinceLinebreak%(ed->csd->tabSize);
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "%s", tabBuffer + (nrTabCharsToSkip));
				nrCharsSinceLinebreak += ed->csd->tabSize - nrTabCharsToSkip - 1;
				break;
			default:
				if (currentChar < 0x20)	//ignore control characters
					break;
				currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "%c", currentChar);
				break;
		}
	}

	if (openSpan) {
		currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "</span>");
	}

	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "</div>");

	delete [] tabBuffer;

	//add EndFragment if doing CF_HTML
	endFragment = currentBufferOffset;
	if (addHeader) {
		currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "<!--EndFragment-->\r\n");
	}
	//end EndFragment

	//add closing context
	currentBufferOffset += sprintf(clipbuffer+currentBufferOffset, "</body>\r\n</html>\r\n");
	endHTML = currentBufferOffset;

	//if doing CF_HTML, fill in header data
	if (addHeader) {
		char number[11];
		sprintf(number, "%.10d", startHTML);
		memcpy(clipbuffer + 23, number, 10);
		sprintf(number, "%.10d", endHTML);
		memcpy(clipbuffer + 43, number, 10);
		sprintf(number, "%.10d", startFragment);
		memcpy(clipbuffer + 69, number, 10);
		sprintf(number, "%.10d", endFragment);
		memcpy(clipbuffer + 93, number, 10);
	}
	//end header

	//char text[500];
	//sprintf(text, "HTML: of %d allocated, %d was used", totalBytesNeeded, currentBufferOffset);
	//MessageBox(NULL, (text), NULL, MB_OK);

	GlobalUnlock(hHTMLBuffer);
	ed->hBuffer = hHTMLBuffer;
	ed->bufferSize = currentBufferOffset;
	return true;
}

TCHAR * HTMLExporter::getClipboardType() {
	return CF_HTML;
}
