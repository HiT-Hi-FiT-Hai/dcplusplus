/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "../client/DCPlusPlus.h"

#include <ole2.h>
#include "UPnP.h"
#include <atlconv.h>
#include "../client/Util.h"

UPnP::UPnP( const string theIPAddress, const string theProtocol, const string theDescription, const short thePort )
{
	// need some messy string conversions in here
	// to convert STL::string to BSTR type strings
	// required for the UPnP code.
	// if anyone knows a better way, please tell....

	PortNumber		    = thePort;
	bstrProtocol        = A2BSTR(theProtocol.c_str());
	bstrInternalClient  = A2BSTR(theIPAddress.c_str());
	bstrDescription		= A2BSTR(theDescription.c_str());
	bstrExternalIP		= A2BSTR("");
	pUN = NULL;
}


// Opens the UPnP ports defined when the object was created
HRESULT UPnP::OpenPorts()
{
	CoInitialize(NULL);
	HRESULT hr = CoCreateInstance (__uuidof(UPnPNAT),
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(IUPnPNAT),
		(void**)&pUN);
	if(SUCCEEDED(hr)) {
		IStaticPortMappingCollection * pSPMC = NULL;
		hr = pUN->get_StaticPortMappingCollection (&pSPMC);
		if(SUCCEEDED(hr) && pSPMC) { // see comment in "else"

			if(bstrProtocol && bstrInternalClient && bstrDescription) {
				IStaticPortMapping * pSPM = NULL;
				hr = pSPMC->Add (PortNumber,bstrProtocol,PortNumber,bstrInternalClient,
					VARIANT_TRUE,bstrDescription,&pSPM );
			} else {
				hr = E_OUTOFMEMORY;
			}
		} else {
			hr = E_FAIL;    // work around a known bug here:  in some error 
			// conditions, get_SPMC NULLs out the pointer, but incorrectly returns a success code.
		}
	}
	return hr;
}

// Closes the UPnP ports defined when the object was created
HRESULT UPnP::ClosePorts()
{
	HRESULT hr = E_FAIL;
	HRESULT hr2 = E_FAIL;

	if(bstrProtocol && bstrInternalClient && bstrDescription ) {
		IStaticPortMappingCollection * pSPMC = NULL;
		hr2 = pUN->get_StaticPortMappingCollection (&pSPMC);
		if(SUCCEEDED(hr2) && pSPMC) {
			hr = pSPMC->Remove (PortNumber,bstrProtocol);
			pSPMC->Release();
		}

		SysFreeString (bstrProtocol);
		SysFreeString (bstrInternalClient);
		SysFreeString (bstrDescription);
		SysFreeString (bstrExternalIP);
		CoUninitialize();
	}

	return hr;
}

// Returns the current external IP address
string UPnP::GetExternalIP()
{
	USES_CONVERSION;
	HRESULT hResult;

	IUPnPNAT *pIUN=NULL;
	hResult=CoCreateInstance( CLSID_UPnPNAT,NULL,CLSCTX_INPROC_SERVER, IID_IUPnPNAT, (void **) &pIUN); 
	IStaticPortMappingCollection *pIMaps=NULL;
	hResult=pIUN->get_StaticPortMappingCollection(&pIMaps);

	if(!pIMaps) {
		return Util::emptyString;
	}

	IUnknown *pUnk=NULL;
	hResult=pIMaps->get__NewEnum(&pUnk);

	IEnumVARIANT *pEnumVar=NULL;
	hResult=pUnk->QueryInterface(IID_IEnumVARIANT, (void **)&pEnumVar);

	VARIANT varCurMapping;
	VariantInit(&varCurMapping);
	pEnumVar->Reset();
	// we are only interested in the 1st map... (as going round in a while loop can take ages)
	BSTR bStrExt = NULL;
	pEnumVar->Next(1,&varCurMapping,NULL);
	IStaticPortMapping *pITheMap=NULL;
	IDispatch *pDispMap = V_DISPATCH(&varCurMapping);
	hResult=pDispMap->QueryInterface(IID_IStaticPortMapping, (void **)&pITheMap);
	hResult=pITheMap->get_ExternalIPAddress(&bStrExt);
	SysFreeString(bStrExt);
	VariantClear(&varCurMapping);


	pEnumVar->Release();
	pUnk->Release();
	pIMaps->Release();

	if(bStrExt != NULL) {
		return OLE2A(bStrExt);
	} else {
		return Util::emptyString;
	}
}

UPnP::~UPnP()
{
}

/**
 * @file
 * $Id: UPnP.cpp,v 1.2 2004/09/09 09:27:36 arnetheduck Exp $
 */
