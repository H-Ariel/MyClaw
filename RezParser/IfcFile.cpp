#include "IfcFile.h"


MenuData::MenuData(shared_ptr<Buffer> menuFileStream) {
	menuFileStream->skip(4);

	name = menuFileStream->readString(menuFileStream->read<uint8_t>());

	int numOfImages = 2;
	for (int i = 0; i < numOfImages; i++) {
		images.push_back(menuFileStream->readString(menuFileStream->read<uint8_t>()));

		const uint8_t* p = menuFileStream->getCData() + menuFileStream->getIndex();
		if (p[1] == 0)
			menuFileStream->skip(2);
		else {
			if (p[0] == 0)
				menuFileStream->skip(1);
			if (numOfImages < 4) // 4 images max
				numOfImages += 1;
		}
	}

	if (numOfImages == 2)
		menuFileStream->skip(2);

	magicNumber = menuFileStream->read<uint16_t>();
	menuFileStream->skip(2);

	int itemsCount = menuFileStream->read<uint16_t>();
	subMenus.reserve(itemsCount);
	menuFileStream->skip(4);

	while (itemsCount-- > 0) subMenus.push_back(MenuData(menuFileStream));
}
