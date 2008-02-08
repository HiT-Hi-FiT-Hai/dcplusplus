#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include "tstring.h"

namespace SmartUtil {
tstring cutText(tstring str, unsigned int maxLength);
tstring escapeMenu(tstring str);
}

#endif /*STRINGUTILS_H_*/
