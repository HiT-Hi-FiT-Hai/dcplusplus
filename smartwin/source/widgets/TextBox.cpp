#include "../../include/smartwin/widgets/TextBox.h"

namespace SmartWin {

TextBox::Seed::Seed() : 
	Widget::Seed(WC_EDIT, WS_CHILD | WS_VISIBLE | WS_TABSTOP, WS_EX_CLIENTEDGE),
	font(new Font(DefaultGuiFont))
{
}

void TextBox::create( const Seed & cs )
{
	ControlType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

SmartUtil::tstring TextBox::getLine(int line) {
	SmartUtil::tstring tmp;
	tmp.resize(std::max(2, lineLength(lineIndex(line))));
	
	*reinterpret_cast<WORD*>(&tmp[0]) = static_cast<WORD>(tmp.size());
	tmp.resize(::SendMessage(this->handle(), EM_GETLINE, static_cast<WPARAM>(line), reinterpret_cast<LPARAM>(&tmp[0])));
	return tmp;
}

SmartUtil::tstring TextBox::textUnderCursor(const ScreenCoordinate& p) {
	int i = charFromPos(p);
	int line = lineFromPos(p);
	int c = (i - lineIndex(line)) & 0xFFFF;
	
	SmartUtil::tstring tmp = getLine(line);
	
	SmartUtil::tstring::size_type start = tmp.find_last_of(_T(" <\t\r\n"), c);
	if(start == SmartUtil::tstring::npos)
		start = 0;
	else
		start++;
	
	SmartUtil::tstring::size_type end = tmp.find_first_of(_T(" >\t\r\n"), start + 1);
	if(end == SmartUtil::tstring::npos)
		end = tmp.size();
	
	return tmp.substr(start, end-start);
}

SmartUtil::tstring TextBoxBase::getSelection() const
{
	DWORD start, end;
	this->sendMessage( EM_GETSEL, reinterpret_cast< WPARAM >( & start ), reinterpret_cast< LPARAM >( & end ) );
	SmartUtil::tstring retVal = this->getText().substr( start, end - start );
	return retVal;
}

ScreenCoordinate TextBox::getContextMenuPos() {
	RECT rc;
	::GetClientRect(this->handle(), &rc);
	return ClientCoordinate (Point(rc.right/2, rc.bottom/2), this);
}

}
