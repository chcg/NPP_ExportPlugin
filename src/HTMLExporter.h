#pragma once

#include "Exporter.h"

#define CF_HTML			TEXT("HTML Format")

//size definitions for memory allocation
#define EXPORT_SIZE_HTML_STATIC		(112+143+91+109+24)		//doctype + begin header + default style + second header + footer
#define EXPORT_SIZE_HTML_UTF8		(5)						//UTF8 meta tag
#define EXPORT_SIZE_HTML_1252		(12)					//windows-1252 meta tag
#define EXPORT_SIZE_HTML_STYLE		(75+19)					//bold color bgcolor + font
#define EXPORT_SIZE_HTML_SWITCH		(27)					//<span ...></span>
#define EXPORT_SIZE_HTML_CLIPBOARD	(107+22+20)				//CF_HTML data

struct ExportData;
class Exporter;

class HTMLExporter :
	public Exporter
{
public:
	HTMLExporter(void);
	~HTMLExporter(void);
	bool exportData(ExportData * ed);
	TCHAR * getClipboardType();
};
