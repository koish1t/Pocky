/*  This file is part of Checkpoint
>	Copyright (C) 2017 Bernardo Giordano
>
>   This program is free software: you can redistribute it and/or modify
>   it under the terms of the GNU General Public License as published by
>   the Free Software Foundation, either version 3 of the License, or
>   (at your option) any later version.
>
>   This program is distributed in the hope that it will be useful,
>   but WITHOUT ANY WARRANTY; without even the implied warranty of
>   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
>   GNU General Public License for more details.
>
>   You should have received a copy of the GNU General Public License
>   along with this program.  If not, see <http://www.gnu.org/licenses/>.
>   See LICENSE for information.
*/

#include "gui.h"
#include "error.h"

static Info info;
static Error error;
static Clickable* buttonDisable;
static Clickable* buttonApply;
static Scrollable* directoryList;

size_t getScrollableIndex(void)
{
	return directoryList->getIndex();
} 

Gui::Gui(void)
{
	index = 0;
	page = 0;
	bottomScrollEnabled = false;
	info.init("", "", 0);
	error.init(0, "");
	buttonDisable = new Clickable(204, 102, 110, 54, WHITE, bottomScrollEnabled ? BLACK : GREYISH, "Disable Mod", true);
	buttonApply = new Clickable(204, 158, 110, 54, WHITE, bottomScrollEnabled ? BLACK : GREYISH, "Apply Mod", true);
	directoryList = new Scrollable(6, 102, 196, 110, 5);
}

void Gui::createInfo(std::string title, std::string message)
{
	error.resetTtl();
	info.init(title, message, 500);
}

void Gui::createError(Result res, std::string message)
{
	info.resetTtl();
	error.init(res, message);
}

bool Gui::getBottomScroll(void)
{
	return bottomScrollEnabled;
}

size_t Gui::getNormalizedIndex(void)
{
	return index + page*rowlen*collen;
}

void Gui::setBottomScroll(bool enable)
{
	bottomScrollEnabled = enable;
}

void Gui::updateButtonsColor(void)
{
	if (bottomScrollEnabled)
	{
		buttonDisable->setColors(WHITE, BLACK);
		buttonApply->setColors(WHITE, BLACK);
	}
	else
	{
		buttonDisable->setColors(WHITE, GREYISH);
		buttonApply->setColors(WHITE, GREYISH);		
	}
}

void Gui::updateSelector(void)
{
	if (!getBottomScroll())
	{
		const size_t entries = rowlen * collen;
		const size_t maxentries = (getTitlesCount() - page*entries) > entries ? entries : getTitlesCount() - page*entries;
		const size_t maxpages = getTitlesCount() / entries + 1;
		calculateIndex(index, page, maxpages, maxentries, entries, collen);

		directoryList->resetIndex();
	}
	else
	{
		directoryList->updateSelection();
	}
}

void Gui::drawSelector(void)
{
	static const int w = 2;
	const int x = getSelectorX(index);
	const int y = getSelectorY(index);
	pp2d_draw_rectangle(         x,          y, 50,       50, RGBA8(255, 255, 255, 200)); 
	pp2d_draw_rectangle(         x,          y, 50,    w + 1, RED);
	pp2d_draw_rectangle(         x,      y + w,  w, 50 - 2*w, RED); 
	pp2d_draw_rectangle(x + 50 - w,      y + w,  w, 50 - 2*w, RED);
	pp2d_draw_rectangle(         x, y + 50 - w, 50,        w, RED); 
	
}

int Gui::getSelectorX(size_t index)
{
	return 50*((index % (rowlen*collen)) % collen);
}

int Gui::getSelectorY(size_t index)
{
	return 20 + 50*((index % (rowlen*collen)) / collen);
}

void Gui::draw(void)
{
	const size_t entries = rowlen * collen;
	const size_t max = (getTitlesCount() - page*entries) > entries ? entries : getTitlesCount() - page*entries;
	
	char version[10];
	sprintf(version, "v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
	float versionLen = pp2d_get_text_width(version, 0.45f, 0.45f);
	float smLen = pp2d_get_text_width("Pocky", 0.50f, 0.50f);
	
	const Mode_t mode = getMode();
	
	pp2d_begin_draw(GFX_TOP, GFX_LEFT);
		pp2d_draw_rectangle(0, 0, 400, 19, COLOR_BARS);
		pp2d_draw_rectangle(0, 221, 400, 19, COLOR_BARS);
		
		pp2d_draw_text(4, 3, 0.45f, 0.45f, GREYISH, getTime().c_str());
		pp2d_draw_text(TOP_WIDTH - 4 - versionLen, 3, 0.45f, 0.45f, GREYISH, version);
		pp2d_draw_text(TOP_WIDTH - 6 - versionLen - smLen, 2, 0.50f, 0.50f, WHITE, "Pocky");
		
		for (size_t k = page*entries; k < page*entries + max; k++)
		{
			pp2d_draw_texture(getTextureId(k), getSelectorX(k) + 1, getSelectorY(k) + 1);			
		}
		
		if (getTitlesCount() > 0)
		{
			drawSelector();
		}
		
		static const float p1width = pp2d_get_text_width("\uE000 to select game. \uE006 to move.", 0.47f, 0.47f);
		static const float p2width = pp2d_get_text_width("extdata", 0.47f, 0.47f);
		static const float p3width = pp2d_get_text_width(". \uE006 to move.", 0.47f, 0.47f);
		static const float border = (TOP_WIDTH - p1width - p2width - p3width) / 2;
		pp2d_draw_text(border, 224, 0.47f, 0.47f, WHITE, "\uE000 to select game. \uE006 to move.");
		pp2d_draw_text(border + p1width, 224, 0.47f, 0.47f, getMode() == MODE_MOD ? WHITE : RED, "extdata");
		pp2d_draw_text(border + p1width + p2width, 224, 0.47f, 0.47f, WHITE, ". \uE006 to move.");
		
		info.draw();
		error.draw();
		
		pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
		pp2d_draw_rectangle(0, 0, 320, 19, COLOR_BARS);
		pp2d_draw_rectangle(0, 221, 320, 19, COLOR_BARS);
		
		if (getTitlesCount() > 0)
		{
			Title title;
			getTitle(title, index + page*entries);
			
			directoryList->flush();
			std::vector<std::u16string> dirs = mode == MODE_MOD ? title.getDirectories() : title.getExtdatas();
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
			
			for (size_t i = 0; i < dirs.size(); i++)
			{
				directoryList->addCell(WHITE, bottomScrollEnabled ? BLUE : GREYISH, convert.to_bytes(dirs.at(i)));
				if (i == directoryList->getIndex())
				{
					directoryList->invertCellColors(i);
				}
			}
			
			pp2d_draw_text(4, 1, 0.6f, 0.6f, WHITE, title.getShortDescription().c_str());
			pp2d_draw_text_wrap(4, 27, 0.55f, 0.55f, GREYISH, 240, title.getLongDescription().c_str());
			
			float longDescrHeight = pp2d_get_text_height_wrap(title.getLongDescription().c_str(), 0.55f, 0.55f, 240);
			pp2d_draw_text(4, 31 + longDescrHeight, 0.5f, 0.5f, GREYISH, "ID:");
			pp2d_draw_textf(25, 31 + longDescrHeight, 0.5f, 0.5f, WHITE, "%08X", title.getLowId());
			pp2d_draw_text(4, 47 + longDescrHeight, 0.5f, 0.5f, GREYISH, "Mediatype:");
			pp2d_draw_textf(75, 47 + longDescrHeight, 0.5f, 0.5f, WHITE, "%s", title.getMediatypeString().c_str());
			
			pp2d_draw_rectangle(260, 27, 52, 52, BLACK);
			pp2d_draw_texture(title.getTextureId(), 262, 29);
			
			pp2d_draw_rectangle(4, 100, 312, 114, GREYISH);
			pp2d_draw_rectangle(6, 102, 308, 110, COLOR_BARS);
			pp2d_draw_rectangle(202, 102, 2, 110, GREYISH);
			pp2d_draw_rectangle(204, 156, 110, 2, GREYISH);

			directoryList->draw();
			buttonDisable->draw();
			buttonApply->draw();
		}
		
		pp2d_draw_text_center(GFX_BOTTOM, 224, 0.46f, 0.46f, WHITE, "Press \uE073 to exit.");
	pp2d_end_draw();
}

bool Gui::isDisableReleased(void)
{
	return buttonDisable->isReleased() && bottomScrollEnabled;
}

bool Gui::isApplyReleased(void)
{
	return buttonApply->isReleased() && bottomScrollEnabled;
}

void Gui::resetIndex(void)
{
	index = 0;
	page = 0;
}

void Gui::drawButtons(void)
{
    buttonDisable->draw();
    buttonApply->draw();
}