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
	int currentBufferOffset = 0;
	
	int totalBytesNeeded = 1; 	// Zero terminated
	totalBytesNeeded += ed->csd->nrChars;

	
	HGLOBAL hTXTBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, totalBytesNeeded);
	if (hTXTBuffer == nullptr)
	{
		return false;
	}

	char * clipbuffer = (char *)GlobalLock(hTXTBuffer);
	if (clipbuffer == nullptr)
	{
		GlobalFree(hTXTBuffer);
		return false;
	}

	clipbuffer[0] = 0;

	
//-------Dump text to TXT
	int i = 0;
	for(i = 0; i < ed->csd->nrChars; i++) {
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
