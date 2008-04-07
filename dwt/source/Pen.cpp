#include "../include/smartwin/resources/Pen.h"

namespace SmartWin {

Pen::Pen(HPEN h, bool own) : ResourceType(h, own) { }

Pen::Pen(COLORREF color, PenStyle style, int width) : ResourceType(::CreatePen(style, width, color), true) { }

}
