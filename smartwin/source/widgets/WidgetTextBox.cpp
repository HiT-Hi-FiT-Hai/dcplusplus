#include "../../include/smartwin/widgets/WidgetTextBox.h"

namespace SmartWin {

const WidgetTextBox::Seed & WidgetTextBox::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = _T("Edit");
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_WANTRETURN;
		d_DefaultValues.exStyle = WS_EX_CLIENTEDGE;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetTextBox::create( const WidgetTextBox::Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	setFont( cs.font );
}

SmartUtil::tstring WidgetTextBox::getLine(int line) {
	SmartUtil::tstring tmp;
	tmp.resize(std::max(2, lineLength(lineIndex(line))));
	
	*reinterpret_cast<WORD*>(&tmp[0]) = static_cast<WORD>(tmp.size());
	tmp.resize(::SendMessage(this->handle(), EM_GETLINE, static_cast<WPARAM>(line), reinterpret_cast<LPARAM>(&tmp[0])));
	return tmp;
}

SmartUtil::tstring WidgetTextBox::textUnderCursor(const SmartWin::Point& p) {
	int i = charFromPos(p);
	int line = lineFromPos(p);
	int c = i - lineIndex(line);
	
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

SmartUtil::tstring WidgetTextBoxBase::getSelection() const
{
	DWORD start, end;
	this->sendMessage( EM_GETSEL, reinterpret_cast< WPARAM >( & start ), reinterpret_cast< LPARAM >( & end ) );
	SmartUtil::tstring retVal = this->getText().substr( start, end - start );
	return retVal;
}

Point WidgetTextBox::getContextMenuPos() {
	RECT rc;
	::GetClientRect(this->handle(), &rc);
	POINT pt = { rc.right/2, rc.bottom/2};
	this->clientToScreen(pt);
	return pt;
}

}
