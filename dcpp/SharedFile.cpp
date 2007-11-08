/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "SharedFile.h"

namespace dcpp {

SharedFile::FileData::FileData(const string& filename) : f(filename, File::RW, File::OPEN | File::CREATE) {
	
}

SharedFile::SharedFile(const string& filename, const Segment& segment_, int64_t totalSize) : segment(segment_), pos(0) {
	file = FileDataPtr(new FileData(filename));
	if(totalSize != -1) {
		file->f.setSize(totalSize);
	}
}

size_t SharedFile::write(const void* buf, size_t len) throw(Exception) {
	Lock l(file->cs);
	file->f.setPos(segment.getStart() + pos);
	size_t n = file->f.write(buf, len);
	pos += n;
	return n;
}

size_t SharedFile::read(void* buf, size_t& len) throw(Exception) {
	Lock l(file->cs);
	file->f.setPos(segment.getStart() + pos);
	size_t n = file->f.read(buf, len);
	pos += n;
	return n;
}

size_t SharedFile::flush() throw(Exception) {
	Lock l(file->cs);
	return file->f.flush();
}

}
