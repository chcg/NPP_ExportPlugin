#include "stdafx.h"
#include "Scintilla.h"
#include "ExportStructs.h"
#include "Exporter.h"
#include "TXTExporter.h"
#include <stdio.h>

TXTExporter::TXTExporter(void) {
	setClipboardID( CF_TEXT );
	//no need to register
}

TXTExporter::~TXTExporter(void) {
}

bool TXTExporter::exportData(ExportData * ed) {
	//estimate buffer size needed
	char * buffer = ed->csd->dataBuffer;
	bool isUnicode = (ed->csd->currentCodePage == SC_CP_UTF8);

	int totalBytesNeeded = ed->csd->nrChars;
	
	unsigned char testChar = 0;


	int currentBufferOffset = 0;
	HGLOBAL hTXTBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, totalBytesNeeded);
	char * clipbuffer = (char *)GlobalLock(hTXTBuffer);
	clipbuffer[0] = 0;

	
//-------Dump text to TXT
	for(int i = 0; i < ed->csd->nrChars; i++) {
		clipbuffer[i] = buffer[(i*2)];
	}

	currentBufferOffset += i;

	//char text[500];
	//sprintf(text, "TXT: of %d allocated, %d was used", totalBytesNeeded, currentBufferOffset);
	//MessageBox(NULL, (text), NULL, MB_OK);

	GlobalUnlock(hTXTBuffer);
	ed->hBuffer = hTXTBuffer;
	ed->bufferSize = currentBufferOffset;
	return true;
}

TCHAR * TXTExporter::getClipboardType() {
	return TEXT("Text");
}
