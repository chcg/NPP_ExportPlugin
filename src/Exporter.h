#pragma once

struct ExportData;

class Exporter
{
public:
	Exporter(void) : _clipboardId(0) {};
	~Exporter(void);
	virtual bool exportData(ExportData * ed) { return false; };
	virtual TCHAR * getClipboardType() { return NULL; };
	int getClipboardID() { return _clipboardId; };
protected:
	UINT _clipboardId;

	void setClipboardID(UINT id) {
		_clipboardId = id;
	}
};
