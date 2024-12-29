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

#ifndef FSSTREAM_H
#define FSSTREAM_H

#include "common.h"

u64 getFileSize(FS_Archive archive, std::u16string path);
bool fileExist(FS_Archive archive, std::u16string path);

Result copyFile(FS_Archive srcArch, FS_Archive dstArch, std::u16string srcPath, std::u16string dstPath);
Result readFile(FS_Archive archive, u8* buf, std::u16string path);
Result writeFile(FS_Archive archive, u8* buf, std::u16string path, u64 size);

Result createDirectory(FS_Archive archive, std::u16string path);
bool directoryExist(FS_Archive archive, std::u16string path);
Result copyDirectory(FS_Archive srcArch, FS_Archive dstArch, std::u16string srcPath, std::u16string dstPath);

void applyMod(size_t index);
Result disableMod(size_t index);

class FSStream
{
public:
	FSStream(FS_Archive archive, std::u16string path, u32 flags);
	FSStream(FS_Archive archive, std::u16string path, u32 flags, u64 size);
	
	Result close(void);
	bool getLoaded(void);
	Result getResult(void);
	u32 getSize(void);
	
	Result read(void *buf);
	Result write(void *buf);

private:
	Handle handle;
	u64 size;
	Result res;
	bool loaded;
};

#endif