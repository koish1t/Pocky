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

#include "fsstream.h"

FSStream::FSStream(FS_Archive archive, std::u16string path, u32 flags)
{
	loaded = false;
	size = 0;
	
	res = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path.data()), flags, 0);
	if (R_SUCCEEDED(res))
	{
		FSFILE_GetSize(handle, &size);
		loaded = true;		
	}
}

FSStream::FSStream(FS_Archive archive, std::u16string path, u32 flags, u64 _size)
{
	loaded = false;
	size = _size;
	
	res = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path.data()), flags, 0);
	if (R_FAILED(res))
	{
		res = FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, path.data()), flags, size);
		if (R_SUCCEEDED(res))
		{
			res = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, path.data()), flags, 0);
			if (R_SUCCEEDED(res))
			{
				loaded = true;
			}
		}
	}	
}

Result FSStream::close(void)
{
	res = FSFILE_Close(handle);
	return res;
}

bool FSStream::getLoaded(void)
{
	return loaded;
}

Result FSStream::getResult(void)
{
	return res;
}

u32 FSStream::getSize(void)
{
	return size;
}

Result FSStream::read(void *buf)
{
	return FSFILE_Read(handle, NULL, 0, buf, size);
}

Result FSStream::write(void *buf)
{
	return FSFILE_Write(handle, NULL, 0, buf, size, FS_WRITE_FLUSH);
}

u64 getFileSize(FS_Archive archive, std::u16string path)
{
	u64 size = 0;
	FSStream stream(archive, path, FS_OPEN_READ);
	if (stream.getLoaded())
	{
		size = stream.getSize();
	}
	stream.close();
	return size;
}

bool fileExist(FS_Archive archive, std::u16string path)
{
	FSStream stream(archive, path, FS_OPEN_READ);
	bool exist = stream.getLoaded();
	stream.close();
	return exist;
}

Result copyFile(FS_Archive srcArch, FS_Archive dstArch, std::u16string srcPath, std::u16string dstPath)
{
	Result res;
	u64 size = getFileSize(srcArch, srcPath);
	if (size == 0)
	{
		return -1;
	}
	
	u8 *buf = new u8[size];
	res = readFile(srcArch, buf, srcPath);
	if (R_FAILED(res))
	{
		return res;
	}
	
	res = writeFile(dstArch, buf, dstPath, size);
	
	delete[] buf;
	return res;
}

Result copyDirectory(FS_Archive srcArch, FS_Archive dstArch, std::u16string srcPath, std::u16string dstPath)
{
	Result res = 0;
	bool quit = false;
	Directory items(srcArch, srcPath);
	
	if (!items.getLoaded())
	{
		return items.getError();
	}
	
	for (size_t i = 0, sz = items.getCount(); i < sz && !quit; i++)
	{
		std::u16string newsrc = srcPath + items.getItem(i);
		std::u16string newdst = dstPath + items.getItem(i);
		
		if (items.isFolder(i))
		{
			res = createDirectory(dstArch, newdst);
			
			if (R_FAILED(res))
			{
				quit = true;
				createError(res, "Failed to create destination directory.");
			}
			else
			{
				newsrc += u8tou16("/");
				newdst += u8tou16("/");
				res = copyDirectory(srcArch, dstArch, newsrc, newdst);
			}
		}
		else
		{
			res = copyFile(srcArch, dstArch, newsrc, newdst);
			if (R_FAILED(res))
			{
				quit = true;
				createError(res, "Failed to copy file.");
			}
		}
	}
	
	return res;
}

Result readFile(FS_Archive archive, u8* buf, std::u16string path)
{
	Result res;
	FSStream stream(archive, path, FS_OPEN_READ);
	if (stream.getLoaded())
	{
		res = stream.read(buf);
	}
	else
	{
		res = stream.getResult();
	}
	
	stream.close();
	return res;	
}

Result writeFile(FS_Archive archive, u8* buf, std::u16string path, u64 size)
{
	Result res;
	if (fileExist(archive, path))
	{
		res = FSUSER_DeleteFile(archive, fsMakePath(PATH_UTF16, path.data()));
		if (R_FAILED(res))
		{
			return res;
		}
	}
	
	FSStream stream(archive, path, FS_OPEN_WRITE, size);
	if (stream.getLoaded())
	{
		res = stream.write(buf);
	}
	else
	{
		res = stream.getResult();
	}
	
	stream.close();
	return res;
}

Result createDirectory(FS_Archive archive, std::u16string path)
{
	return FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path.data()), 0);
}

bool directoryExist(FS_Archive archive, std::u16string path)
{
	Handle handle;

	if (R_FAILED(FSUSER_OpenDirectory(&handle, archive, fsMakePath(PATH_UTF16, path.data()))))
	{
		return false;
	}

	if (R_FAILED(FSDIR_Close(handle)))
	{
		return false;
	}

	return true;
}

void applyMod(size_t index)
{
	const size_t cellIndex = getScrollableIndex();
	if (cellIndex == 0) 
	{
		return;
	}
	
	Result res = 0;
	Title title;
	getTitle(title, index);
	
	std::u16string lumaBasePath = u8tou16("/luma/titles/");
	char titlePath[34];
	snprintf(titlePath, sizeof(titlePath), "%016llX", title.getId());
	std::u16string lumaTitlePath = lumaBasePath + u8tou16(titlePath) + u8tou16("/");
	
	if (!directoryExist(getArchiveSDMC(), lumaBasePath))
	{
		res = createDirectory(getArchiveSDMC(), lumaBasePath);
		if (R_FAILED(res))
		{
			createError(res, "Failed to create Luma directory.");
			return;
		}
	}
	
	if (directoryExist(getArchiveSDMC(), lumaTitlePath))
	{
		res = FSUSER_DeleteDirectoryRecursively(getArchiveSDMC(), fsMakePath(PATH_UTF16, lumaTitlePath.data()));
		if (R_FAILED(res))
		{
			createError(res, "Failed to clean existing mod files.");
			return;
		}
	}
	
	res = createDirectory(getArchiveSDMC(), lumaTitlePath);
	if (R_FAILED(res))
	{
		createError(res, "Failed to create mod directory.");
		return;
	}
	
	std::u16string srcPath = title.getBackupPath();
	srcPath += u8tou16("/") + u8tou16(getPathFromCell(cellIndex).c_str()) + u8tou16("/");
	
	res = copyDirectory(getArchiveSDMC(), getArchiveSDMC(), srcPath, lumaTitlePath);
	if (R_FAILED(res))
	{
		createError(res, "Failed to copy mod files.");
		return;
	}
	
	createInfo("Success!", getPathFromCell(cellIndex) + " has been applied. Restart game to see changes.");
}

Result disableMod(size_t index)
{
	Result res = 0;
	Title title;
	getTitle(title, index);
	
	std::u16string lumaBasePath = u8tou16("/luma/titles/");
	char titlePath[34];
	snprintf(titlePath, sizeof(titlePath), "%016llX", title.getId());
	std::u16string lumaTitlePath = lumaBasePath + u8tou16(titlePath) + u8tou16("/");
	
	if (directoryExist(getArchiveSDMC(), lumaTitlePath))
	{
		res = FSUSER_DeleteDirectoryRecursively(getArchiveSDMC(), fsMakePath(PATH_UTF16, lumaTitlePath.data()));
		if (R_FAILED(res))
		{
			createError(res, "Failed to remove mod files.");
			return res;
		}
		createInfo("Success!", "Mod has been disabled. Restart game to see changes.");
	}
	else
	{
		createInfo("Notice", "No active mod found for this title.");
	}
	
	return res;
}