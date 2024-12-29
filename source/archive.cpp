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

#include "archive.h"

static FS_Archive archiveSDMC;
static Mode_t mode = MODE_MOD;

Mode_t getMode(void)
{
	return mode;
}

void setMode(Mode_t newmode)
{
	mode = newmode;
}

Result archiveInit(void)
{
	return FSUSER_OpenArchive(&archiveSDMC, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
}

void archiveExit(void)
{
	FSUSER_CloseArchive(archiveSDMC);
}

FS_Archive getArchiveSDMC(void)
{
	return archiveSDMC;
}