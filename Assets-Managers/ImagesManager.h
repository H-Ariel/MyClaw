#pragma once

#include "GameEngine/UIBaseImage.h"
#include "PidPalette.h"


class ImagesManager
{
public:
	ImagesManager(RezArchive* rezArchive);
	~ImagesManager();

	void setPalette(shared_ptr<PidPalette> palette) { _palette = palette; }
	shared_ptr<UIBaseImage> loadImage(const string& path);
	
	void clearLevelImages(const string& prefix);

private:
	shared_ptr<UIBaseImage> loadPidImage(const string& pidPath);
	shared_ptr<UIBaseImage> loadPcxImage(const string& pcxPath);

	map<string, pair<ID2D1Bitmap*, D2D1_POINT_2F>> _loadedBitmaps; // [path]={bitmap,offset}
	shared_ptr<PidPalette> _palette;
	RezArchive* _rezArchive;
};
