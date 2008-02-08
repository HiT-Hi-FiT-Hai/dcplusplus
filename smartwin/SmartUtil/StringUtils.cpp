#include "StringUtils.h"

#include "UtilSystemHeaders.h"

namespace SmartUtil {
	tstring cutText(tstring str, unsigned int maxLength) {
		if(str.length() > maxLength)
			str = str.substr(0, maxLength - 3) + _T("...");
		return str;
	}

	tstring escapeMenu(tstring str) {
		tstring::size_type i = 0;
		while( (i = str.find(_T('&'), i)) != tstring::npos) {
			str.insert(str.begin()+i, 1, _T('&'));
			i += 2;
		}
		return str;
	}
}
