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

#include "util.h"
#include <sys/stat.h>
#include <3ds/services/fs.h>

void servicesExit(void)
{
	archiveExit();
	amExit();
	srvExit();
	hidExit();
	pp2d_exit();
	fsExit();
}

void servicesInit(void)
{
	Result res = 0;
	
	fsInit();
	pp2d_init();
	hidInit();
	srvInit();
	amInit();
	
	res = archiveInit();
	if (R_FAILED(res))
	{
		createError(res, "SDMC archive init failed.");
	}
	
	mkdir("sdmc:/3ds", 777);
	mkdir("sdmc:/3ds/Pocky", 777);
	mkdir("sdmc:/3ds/Pocky/mods", 777);
	
	createLumaTitleFolders();
}

bool checkHigh(u64 id) {
    return (id >> 32) == 0x00040000;
}

void createLumaTitleFolders(void) {
    std::u16string lumaBasePath = u8tou16("/luma/titles/");
    
	// idek why you'd need this, 90% of users use luma lol
    if (!directoryExist(getArchiveSDMC(), lumaBasePath)) {
        Result res = createDirectory(getArchiveSDMC(), lumaBasePath);
        if (R_FAILED(res)) {
            createError(res, "Failed to create Luma directory.");
            return;
        }
    }

    u32 count = 0;
    AM_GetTitleCount(MEDIATYPE_SD, &count);
    
    u64 ids[count];
    AM_GetTitleList(NULL, MEDIATYPE_SD, count, ids);

    for (u32 i = 0; i < count; i++) {
        if (checkHigh(ids[i])) {
            char titlePath[34];
            snprintf(titlePath, sizeof(titlePath), "%016llX", ids[i]);
            std::u16string lumaTitlePath = lumaBasePath + u8tou16(titlePath);
            
            if (!directoryExist(getArchiveSDMC(), lumaTitlePath)) {
                createDirectory(getArchiveSDMC(), lumaTitlePath);
            }
        }
    }

    AM_GetTitleCount(MEDIATYPE_GAME_CARD, &count);
    if (count > 0) {
        AM_GetTitleList(NULL, MEDIATYPE_GAME_CARD, count, ids);
        if (checkHigh(ids[0])) {
            char titlePath[34];
            snprintf(titlePath, sizeof(titlePath), "%016llX", ids[0]);
            std::u16string lumaTitlePath = lumaBasePath + u8tou16(titlePath);
            
            if (!directoryExist(getArchiveSDMC(), lumaTitlePath)) {
                createDirectory(getArchiveSDMC(), lumaTitlePath);
            }
        }
    }
}

bool directoryEqual(FS_Archive archive, const std::u16string& dir1, const std::u16string& dir2) 
{
    Directory d1(archive, dir1);
    Directory d2(archive, dir2);
    
    if (!d1.getLoaded() || !d2.getLoaded()) {
        return false;
    }
    
    if (d1.getCount() != d2.getCount()) {
        return false;
    }
    
    std::vector<std::u16string> items1;
    std::vector<std::u16string> items2;
    
    for (size_t i = 0; i < d1.getCount(); i++) {
        items1.push_back(d1.getItem(i));
        items2.push_back(d2.getItem(i));
    }
    
    std::sort(items1.begin(), items1.end());
    std::sort(items2.begin(), items2.end());
    
    return items1 == items2;
}