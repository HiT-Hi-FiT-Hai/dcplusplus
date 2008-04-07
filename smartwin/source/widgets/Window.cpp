#include "../../include/smartwin/widgets/Window.h"

namespace SmartWin {

Window::Seed::Seed(const SmartUtil::tstring& caption) : BaseType::Seed(caption, 0, 0) {
}

void Window::create(const Seed& cs) {
	BaseType::create(cs);
}

}
