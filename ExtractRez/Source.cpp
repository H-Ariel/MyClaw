#include "RezParser/RezArchive.h"
#include "RezParser/PidFile.h"
#include "RezParser/PcxFile.h"
#include "RezParser/AniFile.h"
#include "RezParser/WwdFile.h"
#include "RezParser/XmiFile.h"
#include "RezParser/IfcFile.h"
#include "lodepng.h"
#include "json.hpp"

#define DIR_OUT_NAME	"C:/claw-assets/"

using nlohmann::json;



static void make_dir(const string& path)
{
	static vector<string> foldersCreated;
	if (!FindInArray(foldersCreated, path))
	{
		foldersCreated.push_back(path);
		string fullpath = "\"" DIR_OUT_NAME + path + '"';
		system(("if not exist " + fullpath + " mkdir " + fullpath).c_str());
	}
}


static void saveRaw(const string& fpath, const DynamicArray<uint8_t>& data)
{
	if (FILE* file = fopen((DIR_OUT_NAME + fpath).c_str(), "wb"))
	{
		fwrite(data.data(), sizeof(data[0]), data.size(), file);
		fclose(file);
	}
}
static void saveRaw(RezFile* rFile)
{
	saveRaw(rFile->getFullPath(), rFile->getFileData());
}
static void savePidFile(RezFile* file, WapPal* pal)
{
	WapPid pid(file->getFileData(), pal);
	vector<unsigned char> rgba = vector<unsigned char>(
		(unsigned char*)pid.colors.data(),
		(unsigned char*)pid.colors.data() + pid.colors.size() * sizeof(pid.colors[0])
	);
	lodepng::encode(DIR_OUT_NAME + file->getFullPath() + ".png", rgba, pid.width, pid.height);
}
static void savePcxFile(RezFile* file)
{
	PcxFile pcx(file->getFileReader());
	vector<unsigned char> rgba = vector<unsigned char>(
		(unsigned char*)pcx.colors.data(),
		(unsigned char*)pcx.colors.data() + pcx.colors.size() * sizeof(pcx.colors[0])
	);
	lodepng::encode(DIR_OUT_NAME + file->getFullPath() + ".png", rgba, pcx.width, pcx.height);
}
static void saveXmiFile(RezFile* file)
{
	saveRaw(file->getFullPath() + ".midi", MidiFile(file->getFileData()).data);
}
static void saveAniFile(RezFile* file)
{
	auto b = file->getFileReader();
	WapAni ani(b);

	json j;
	vector<json> j_animationFrames;

	for (const auto& f : ani.animationFrames)
	{
		json fj;

		fj["duration"] = f.duration;
		fj["eventFilePath"] = f.eventFilePath;
		fj["imageFileId"] = f.imageFileId;
		fj["useSoundFile"] = f.useSoundFile;

		j_animationFrames.push_back(fj);
	}

	j["animationFrames"] = j_animationFrames;
	j["imageSetPath"] = ani.imageSetPath;

	ofstream(DIR_OUT_NAME + file->getFullPath() + ".json") << j.dump(2);
}
static void saveWwdFile(RezFile* file)
{
	json j;
	WapWwd wwd(file->getFileReader());

	json j_properties;
	vector<json> j_planes;

	//j_properties["flags"] = wwd.flags;
	j_properties["levelName"] = wwd.levelName;
	//j_properties["author"] = wwd.author;
	//j_properties["birth"] = wwd.birth;
	//j_properties["rezFile"] = wwd.rezFile;
	j_properties["imageDirectoryPath"] = wwd.imageDirectoryPath;
	j_properties["rezPalettePath"] = wwd.rezPalettePath;
	j_properties["startX"] = wwd.startX;
	j_properties["startY"] = wwd.startY;
	//j_properties["launchApp"] = wwd.launchApp;
	//j_properties["imageSet"] = wwd.imageSet;
	//j_properties["prefix"] = wwd.prefix;

	json j_prefix_imageSet;
	for (int i = 0; i < 4; i++)
		j_prefix_imageSet[wwd.prefix[i]] = wwd.imageSet[i];
	j_properties["prefix_imageSet"] = j_prefix_imageSet;


	for (auto& plane : wwd.planes)
	{
		json j_plane;
		json j_plane_props;

		j_plane["tilesWide"] = plane.tilesWide;
		j_plane["tilesHigh"] = plane.tilesHigh;
		j_plane["tiles"] = plane.tiles;
		j_plane["imageSets"] = plane.imageSets;

		j_plane_props["flags"] = plane.flags;
		j_plane_props["name"] = plane.name;
		j_plane_props["pixelWidth"] = plane.pixelWidth;
		j_plane_props["pixelHeight"] = plane.pixelHeight;
		j_plane_props["tilePixelWidth"] = plane.tilePixelWidth;
		j_plane_props["tilePixelHeight"] = plane.tilePixelHeight;
		j_plane_props["tilesOnAxisX"] = plane.tilesOnAxisX;
		j_plane_props["tilesOnAxisY"] = plane.tilesOnAxisY;
		j_plane_props["movementPercentX"] = plane.movementPercentX;
		j_plane_props["movementPercentY"] = plane.movementPercentY;
		j_plane_props["fillColor"] = plane.fillColor;
		j_plane_props["coordZ"] = plane.coordZ;

		// TODO: convert to json: `plane.objects`, etc.

		j_plane["properties"] = j_plane_props;

		j_planes.push_back(j_plane);
	}

	j["properties"] = j_properties;
	j["planes"] = j_planes;

	ofstream(DIR_OUT_NAME + file->getFullPath() + ".json") << j.dump(2);
}
static json menuDataToJson(MenuData menuData) {
	vector<json> subMenus;
	for (const MenuData m : menuData.subMenus)
		subMenus.push_back(menuDataToJson(m));

	return json({
		{ "name", menuData.name },
		{ "images", menuData.images },
		{ "magicNumber", menuData.magicNumber },
		{ "subMenus", subMenus }
	});
}
static void saveIfcFile(RezFile* file) {
	ofstream(DIR_OUT_NAME + file->getFullPath() + ".json") << menuDataToJson(MenuData(file->getFileReader())).dump(2);
}

static set<string> notImpletedTypes;

static void loadDirectory(RezDirectory* dir, WapPal* pal)
{
	make_dir(dir->getFullPath());

	// load directories:
	for (const auto& [name, subdir] : dir->rezDirectories)
		loadDirectory(subdir, pal);

	// load files:
	for (auto& [name, file] : dir->rezFiles)
	{
		try
		{
			if (strcmp(file->extension, "PID") == 0) savePidFile(file, pal);
			else if (strcmp(file->extension, "PCX") == 0) savePcxFile(file);
			else if (strcmp(file->extension, "XMI") == 0) saveXmiFile(file);
			else if (strcmp(file->extension, "PAL") == 0) saveRaw(file);
			else if (strcmp(file->extension, "WAV") == 0) saveRaw(file);
			else if (strcmp(file->extension, "TXT") == 0) saveRaw(file);
			else if (strcmp(file->extension, "ANI") == 0) saveAniFile(file);
			else if (strcmp(file->extension, "WWD") == 0) saveWwdFile(file);
			else if (strcmp(file->extension, "IFC") == 0) saveIfcFile(file);
			else {
				notImpletedTypes.insert(file->extension);
				saveRaw(file);
			}
		}
		catch (const Exception& ex)
		{
			cout << "Error:"
				<< "\n  msg: " << ex.message
				<< "\n  file-path: \"" << file->getFullPath() << '"' << endl;
		}
	}
}

static void loadLevel(RezArchive* rez, int l)
{
	cout << "load level " << l << endl;
	string root = "LEVEL" + to_string(l);
	WapPal pal(rez->getFile(root + "/PALETTES/MAIN.PAL")->getFileData());
	loadDirectory(rez->getDirectory(root), &pal);
}


int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // check for memory leaks
#endif

	cout << "load REZ file" << endl;
	RezArchive rez("CLAW.REZ");

	WapPal pal(rez.getFile("LEVEL1/PALETTES/MAIN.PAL")->getFileData());
	cout << "load CLAW" << endl; loadDirectory(rez.getDirectory("CLAW"), &pal);
	//cout << "load GAME" << endl; loadDirectory(rez.getDirectory("GAME"), &pal);
	//cout << "load STATES" << endl; loadDirectory(rez.getDirectory("STATES"), &pal);
	//for (int i = 1; i <= 14; loadLevel(&rez, i++));

	cout << "not impleted types:" << endl;
	for (auto& i : notImpletedTypes)
		cout << i << ", ";

	return 0;
}
