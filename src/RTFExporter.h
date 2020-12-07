#pragma once

#include "Exporter.h"

#define CF_RTF 			TEXT("Rich Text Format")

//size definitions for memory allocation
constexpr auto EXPORT_SIZE_RTF_STATIC = (33 + 11 + 5 + 12 + 5 + 22 + 3);	//header + fonttbl + fonttbl end + colortbl + colortbl end + default style + eof
constexpr auto EXPORT_SIZE_RTF_STYLE = (11 + 27 + 27);						//font decl + color decl fg + color decl bg
constexpr auto EXPORT_SIZE_RTF_SWITCH = (39);								// '\f127\fs56\highlight254\cf255\b0\i0\ul0 '

struct ExportData;
class Exporter;

union utf16struct {
	short value;
	struct {
		char byte1;
		char byte2;
	} bytes;
};

typedef utf16struct utf16;

class RTFExporter :
	public Exporter
{
public:
	RTFExporter(void);
	~RTFExporter(void);
	bool exportData(ExportData * ed);
	TCHAR * getClipboardType();
};
