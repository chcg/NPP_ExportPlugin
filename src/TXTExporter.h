#pragma once

#include "Exporter.h"

//size definitions for memory allocation
//NONE

struct ExportData;
class Exporter;

class TXTExporter :
	public Exporter
{
public:
	TXTExporter(void);
	~TXTExporter(void);
	bool exportData(ExportData * ed) override;
	TCHAR * getClipboardType() override;
};
