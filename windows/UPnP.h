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
#pragma once
#include <natupnp.h>

class UPnP
{
public:
	UPnP( const string, const string, const string, const short );
	~UPnP();
	HRESULT OpenPorts();
	HRESULT ClosePorts();
	string UPnP::GetExternalIP();
private:
	int PortNumber; // The Port number required to be opened
	BSTR bstrInternalClient; // Local IP Address
	BSTR bstrDescription;  // name shown in UPnP interfece details
	BSTR bstrProtocol; // protocol (TCP or UDP)
	BSTR bstrExternalIP; // external IP address
	IUPnPNAT* pUN;  // pointer to the UPnPNAT interface
	IStaticPortMappingCollection* pSPMC; // pointer to the collection
	IStaticPortMapping * pSPM; // pointer to the port map
};

/**
 * @file
 * $Id: UPnP.h,v 1.1 2004/09/08 14:27:14 arnetheduck Exp $
 */
