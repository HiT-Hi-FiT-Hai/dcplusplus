#include "../../include/smartwin/widgets/SaveDialog.h"

namespace SmartWin {

bool SaveDialog::open(SmartUtil::tstring& target) {
	OPENFILENAME ofn = { sizeof(OPENFILENAME) }; // common dialog box structure
	fillOFN( ofn, 0 );

	if ( ::GetSaveFileName( & ofn ) ) {
		target = ofn.lpstrFile;
		return true;
	}
	return false;
}

}
