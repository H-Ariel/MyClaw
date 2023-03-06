#pragma once

#include "UIBaseImage.h"
#include "PidPalette.h"


class ImagesManager
{
public:
	ImagesManager(RezArchive* rezArchive);
	~ImagesManager();

	void setPalette(shared_ptr<PidPalette> palette);
	shared_ptr<UIBaseImage> loadImage(string path);
	
	void clearLevelImages(const string& prefix);

private:
	static ID2D1Bitmap* createBitmapFromBuffer(const void* const buffer, uint32_t width, uint32_t height); // `buffer` is RGBA array

	shared_ptr<UIBaseImage> loadPidImage(const string& pidPath);
	shared_ptr<UIBaseImage> loadPcxImage(const string& pcxPath);

	map<string, pair<ID2D1Bitmap*, D2D1_POINT_2F>> _loadedBitmaps; // [path]={bitmap,offset}
	shared_ptr<PidPalette> _palette;
	RezArchive* _rezArchive;
};
