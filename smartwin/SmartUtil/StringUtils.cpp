#include "StringUtils.h"

#include "UtilSystemHeaders.h"

namespace SmartUtil {
	tstring escapeMenu(tstring str) {
		tstring::size_type i = 0;
		while( (i = str.find(_T('&'), i)) != tstring::npos) {
			str.insert(str.begin()+i, 1, _T('&'));
			i += 2;
		}
		return str;
	}
}
