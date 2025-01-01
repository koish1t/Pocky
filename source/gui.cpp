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

Gui::Gui(void)
{
	index = 0;
	page = 0;
	bottomScrollEnabled = false;
	info.init("", "", 0);
	error.init(0, "");
	buttonDisable = new Clickable(20, 185, 135, 40, COLOR_ACCENT, COLOR_ACCENT, "Disable Mod", true);
	buttonApply = new Clickable(165, 185, 135, 40, COLOR_ACCENT, COLOR_ACCENT, "Apply Mod", true);
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
		buttonDisable->setColors(COLOR_ACCENT, COLOR_ACCENT);
		buttonApply->setColors(COLOR_ACCENT, COLOR_ACCENT);
	}
	else
	{
		buttonDisable->setColors(COLOR_ACCENT, COLOR_ACCENT);
		buttonApply->setColors(COLOR_ACCENT, COLOR_ACCENT);		
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
	const size_t max = getTitlesCount(); 
	
	pp2d_begin_draw(GFX_TOP, GFX_LEFT);
	pp2d_draw_rectangle(0, 0, 400, 240, COLOR_BG);

	if (getTitlesCount() > 0)
	{
		Title title;
		getTitle(title, index + page*entries);
		
		pp2d_draw_texture_scale(getTextureId(index + page*entries), 20, 20, 2.0f, 2.0f);
		pp2d_draw_text(140, 30, 0.8f, 0.8f, WHITE, title.getShortDescription().c_str());
		pp2d_draw_textf(140, 60, 0.5f, 0.5f, GREYISH, "Game ID: %08X", title.getLowId());
		pp2d_draw_textf(140, 80, 0.5f, 0.5f, GREYISH, "Media: %s", title.getMediatypeString().c_str());
		
		if (title.hasActiveMod())
		{
			pp2d_draw_rectangle(140, 100, 200, 30, COLOR_ACCENT);
			pp2d_draw_text(150, 108, 0.5f, 0.5f, WHITE, "Active Mod Installed");
		}
		
		pp2d_draw_rectangle(0, 160, 400, 80, COLOR_ACCENT);
		static int scrollOffset = 0;  
		static size_t lastIndex = 0;

		if (lastIndex != (index + page*entries)) {
			int targetOffset = (index + page*entries) * 60 - 170;
			scrollOffset = targetOffset;
			lastIndex = index + page*entries;
		}

		scrollOffset = std::max(0, scrollOffset);
		scrollOffset = std::min(scrollOffset, static_cast<int>((getTitlesCount() * 60) - 380));

		for (size_t k = 0; k < max; k++)
		{
			int x = 20 - scrollOffset + (k * 60);
			
			if (x >= -50 && x <= 400) 
			{
				Title currentTitle;
				getTitle(currentTitle, k);
				
				if (k == index + page*entries) {
					pp2d_draw_rectangle(x-2, 170, 54, 54, COLOR_SELECTED);
					pp2d_draw_rectangle(x-2, 224, 54, 6, COLOR_ACTIVE_MOD);
				}
				
				pp2d_draw_texture(getTextureId(k), x, 172);
				
				if (currentTitle.hasActiveMod()) {
					pp2d_draw_rectangle(x+40, 172, 8, 8, COLOR_ACTIVE_MOD);
				}
			}
		}
	}

	pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
	pp2d_draw_rectangle(0, 0, 320, 240, COLOR_BG);
	
	if (getTitlesCount() > 0)
	{
		Title title;
		getTitle(title, index + page*entries);
		
		pp2d_draw_rectangle(0, 0, 320, 40, COLOR_ACCENT);
		pp2d_draw_text(10, 12, 0.7f, 0.7f, WHITE, "Available Mods");
		
		std::vector<std::u16string> dirs = title.getDirectories();
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
		
        std::u16string activeModDir = title.getActiveModPath();
        
        int visibleItems = 4;
        int maxIndex = std::max(0, (int)dirs.size() - 1);
        int currentIndex = std::min((int)directoryList->getIndex(), maxIndex);
        directoryList->setIndex(currentIndex); 
        
        int scrollOffset = std::max(0, currentIndex - 1);
        int startIdx = scrollOffset;
        int endIdx = std::min(startIdx + visibleItems, (int)dirs.size());
        
        for (int i = startIdx; i < endIdx; i++)
        {
            bool isSelected = i == directoryList->getIndex();
            bool isActive = dirs[i] == activeModDir;
            int yPos = 50 + ((i - startIdx) * 35); 
            
            pp2d_draw_rectangle(10, yPos, 300, 30, 
                              isSelected ? COLOR_SELECTED : COLOR_LIST_BG);
            
            if (isSelected) {
                pp2d_draw_rectangle(10, yPos, 4, 30, COLOR_ACTIVE_MOD);
            }
            
            if (isActive && i > 0) {
                pp2d_draw_text(280, yPos + 8, 0.5f, 0.5f, COLOR_ACTIVE_MOD, "●");
            }
            
            pp2d_draw_text(20, yPos + 8, 0.5f, 0.5f, WHITE, 
                          convert.to_bytes(dirs.at(i)).c_str());
        }
        
        // draws the arrows, I'm personally gonna keep this commented out for public releases but uncomment it if you wanna.
		/*
        if (startIdx > 0) {
            pp2d_draw_text(300, 45, 0.5f, 0.5f, WHITE, "▲");
        }
        if (endIdx < dirs.size()) {
            pp2d_draw_text(300, 155, 0.5f, 0.5f, WHITE, "▼");
        }
		*/
		
		pp2d_draw_rectangle(0, 180, 320, 60, COLOR_ACCENT);
		
		if (bottomScrollEnabled) {
			pp2d_draw_rectangle(20, 185, 135, 40, RGBA8(255, 255, 255, 30));
			pp2d_draw_rectangle(165, 185, 135, 40, RGBA8(255, 255, 255, 30));
		}
		
		buttonDisable->draw();
		buttonApply->draw();
		
		pp2d_draw_text(10, 227, 0.45f, 0.45f, RGBA8(255, 255, 255, 180), 
					  "\uE000 Select   \uE001 Back   \uE006 Browse");
	}
	
	info.draw();
	error.draw();
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

size_t Gui::getScrollableIndex(void)
{
    return directoryList->getIndex();
}