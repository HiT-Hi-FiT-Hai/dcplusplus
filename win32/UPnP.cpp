/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdafx.h"

#include <ole2.h>
#include "UPnP.h"
#include <dcpp/Util.h>
#include <dcpp/Text.h>

UPnP::UPnP(const string theIPAddress, const string theProtocol, const string theDescription, const unsigned short thePort) {
	// need some messy string conversions in here
	// to convert STL::string to BSTR type strings
	// required for the UPnP code.
	// if anyone knows a better way, please tell....

	PortsAreOpen		= false;

	PortNumber		    = thePort;
	bstrProtocol        = SysAllocString(Text::toT(theProtocol).c_str());
	bstrInternalClient  = SysAllocString(Text::toT(theIPAddress).c_str());
	bstrDescription		= SysAllocString(Text::toT(theDescription).c_str());
	bstrExternalIP		= SysAllocString(L"");
	pUN = NULL;
}

// Opens the UPnP ports defined when the object was created
HRESULT UPnP::OpenPorts() {
	// Lacking the __uuidof in mingw...
	CLSID upnp;
	OLECHAR upnps[] = L"{AE1E00AA-3FD5-403C-8A27-2BBDC30CD0E1}";
	CLSIDFromString(upnps, &upnp);
	IID iupnp;
	OLECHAR iupnps[] = L"{B171C812-CC76-485A-94D8-B6B3A2794E99}";
	CLSIDFromString(iupnps, &iupnp);
	HRESULT hr = CoCreateInstance (upnp,
		NULL,
		CLSCTX_INPROC_SERVER,
		iupnp,
		(void**)&pUN);

	if(SUCCEEDED(hr)) {
		IStaticPortMappingCollection * pSPMC = NULL;
		hr = pUN->get_StaticPortMappingCollection (&pSPMC);

		if(SUCCEEDED(hr) && pSPMC) {
			// see comment in "else"
			if(bstrProtocol && bstrInternalClient && bstrDescription) {
				IStaticPortMapping * pSPM = NULL;
				hr = pSPMC->Add(PortNumber,
					bstrProtocol,
					PortNumber,
					bstrInternalClient,
					VARIANT_TRUE,
					bstrDescription,
					&pSPM
				);

				if(SUCCEEDED(hr)) {
					PortsAreOpen = true;
				}
			} else {
				hr = E_OUTOFMEMORY;
			}
		} else {
			hr = E_FAIL;	// work around a known bug here:  in some error
			// conditions, get_SPMC NULLs out the pointer, but incorrectly returns a success code.
		}
	}
	return hr;
}

// Closes the UPnP ports defined when the object was created
HRESULT UPnP::ClosePorts() {
	if(PortsAreOpen == false) {
		return S_OK;
	}

	HRESULT hr = E_FAIL;
	HRESULT hr2 = E_FAIL;

	if(bstrProtocol && bstrInternalClient && bstrDescription) {
		IStaticPortMappingCollection * pSPMC = NULL;
		hr2 = pUN->get_StaticPortMappingCollection (&pSPMC);

		if(SUCCEEDED(hr2) && pSPMC) {
			hr = pSPMC->Remove(PortNumber, bstrProtocol);
			pSPMC->Release();
		}

	}
	return hr;
}

// Returns the current external IP address
string UPnP::GetExternalIP() {
	HRESULT hr;

	// Check if we opened the desired port, 'cause we use it for getting the IP
	// This shouldn't be a problem because we only try to get the external IP when
	// we opened the mapping
	// This function is not used somewhere else, hence it is "save" to do it like this
	if(!PortsAreOpen) {
		return Util::emptyString;
	}

	// Get the Collection
	IStaticPortMappingCollection *pIMaps = NULL;
	hr = pUN->get_StaticPortMappingCollection(&pIMaps);

	// Check it
	// We also check against that bug mentioned in OpenPorts()
	if(!SUCCEEDED(hr) || !pIMaps ) {
		// Only release when OK
		if(pIMaps != NULL) {
			pIMaps->Release();
		}
		return Util::emptyString;
	}

	// Lets Query our mapping
	IStaticPortMapping *pISM;
	hr = pIMaps->get_Item(
		PortNumber,
		bstrProtocol,
		&pISM
	);

	// Query failed!
	if(!SUCCEEDED(hr)) {
		pIMaps->Release();
		return Util::emptyString;
	}

	// Get the External IP from our mapping
	BSTR bstrExternal = NULL;
	hr = pISM->get_ExternalIPAddress(&bstrExternal);

	// D'OH. Failed
	if(!SUCCEEDED(hr)) {
		pIMaps->Release();
		pISM->Release();
		return Util::emptyString;
	}

	// Check and convert the result
	string tmp;
	if(bstrExternal != NULL) {
		tmp = Text::wideToAcp(bstrExternal);
	} else {
		tmp = Util::emptyString;
	}

	// no longer needed
	SysFreeString(bstrExternal);

	// no longer needed
	pIMaps->Release();
	pISM->Release();

	return tmp;
}

UPnP::~UPnP() {
	if (pUN) {
		pUN->Release();
	}
	SysFreeString(bstrProtocol);
	SysFreeString(bstrInternalClient);
	SysFreeString(bstrDescription);
	SysFreeString(bstrExternalIP);
}
