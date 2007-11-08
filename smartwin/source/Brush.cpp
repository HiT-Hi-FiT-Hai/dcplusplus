#include "../include/smartwin/resources/Brush.h"

namespace SmartWin {

Brush::Brush(HBRUSH h, bool own) : ResourceType(h, own) {
	
}

Brush::Brush(SysColor color) : ResourceType(::GetSysColorBrush(color), false) {
	
}

Brush::Brush(COLORREF color) : ResourceType(::CreateSolidBrush(color), true) {

}

}
