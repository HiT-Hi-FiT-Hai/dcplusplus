/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "CryptoManager.h"

#include "BitInputStream.h"
#include "BitOutputStream.h"
#include "ResourceManager.h"
#include "LogManager.h"

#include <openssl/ssl.h>

#ifdef _WIN32
#include "../bzip2/bzlib.h"
#else
#include <bzlib.h>
#endif

CryptoManager::CryptoManager() 
:	
	clientContext(SSL_CTX_new(TLSv1_client_method())), 
	serverContext(SSL_CTX_new(TLSv1_server_method())), 
	dh(DH_new()), 
	certsLoaded(false), 
	lock("EXTENDEDPROTOCOLABCABCABCABCABCABC"), 
	pk("DCPLUSPLUS" VERSIONSTRING "ABCABC")
{
	static unsigned char dh512_p[] =
	{
		0xDA,0x58,0x3C,0x16,0xD9,0x85,0x22,0x89,0xD0,0xE4,0xAF,0x75,
			0x6F,0x4C,0xCA,0x92,0xDD,0x4B,0xE5,0x33,0xB8,0x04,0xFB,0x0F,
			0xED,0x94,0xEF,0x9C,0x8A,0x44,0x03,0xED,0x57,0x46,0x50,0xD3,
			0x69,0x99,0xDB,0x29,0xD7,0x76,0x27,0x6B,0xA2,0xD3,0xD4,0x12,
			0xE2,0x18,0xF4,0xDD,0x1E,0x08,0x4C,0xF6,0xD8,0x00,0x3E,0x7C,
			0x47,0x74,0xE8,0x33,
	};

	static unsigned char dh512_g[] =
	{
		0x02,
	};

	if(dh) {
		dh->p = BN_bin2bn(dh512_p, sizeof(dh512_p), 0);
		dh->g = BN_bin2bn(dh512_g, sizeof(dh512_g), 0);

		if (!dh->p || !dh->g) {
			DH_free(dh);
			dh = 0;
		} else {
			SSL_CTX_set_tmp_dh(serverContext, dh);
		}
	}
}

CryptoManager::~CryptoManager() {
	if(serverContext)
		SSL_CTX_free(serverContext);
	if(clientContext)
		SSL_CTX_free(clientContext);
	if(dh)
		DH_free(dh);
}


void CryptoManager::loadCertificates() throw() {
	SSL_CTX_set_verify(serverContext, SSL_VERIFY_NONE, 0);
	SSL_CTX_set_verify(clientContext, SSL_VERIFY_NONE, 0);

	if(!SETTING(SSL_CERTIFICATE_FILE).empty()) {
		if(SSL_CTX_use_certificate_file(serverContext, SETTING(SSL_CERTIFICATE_FILE).c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
			LogManager::getInstance()->message("Failed to load certificate file");
			return;
		}
		if(SSL_CTX_use_certificate_file(clientContext, SETTING(SSL_CERTIFICATE_FILE).c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
			LogManager::getInstance()->message("Failed to load certificate file");
			return;
		}
	}

	if(!SETTING(SSL_PRIVATE_KEY_FILE).empty()) {
		if(SSL_CTX_use_PrivateKey_file(serverContext, SETTING(SSL_PRIVATE_KEY_FILE).c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
			LogManager::getInstance()->message("Failed to load private key");
			return;
		}
		if(SSL_CTX_use_PrivateKey_file(clientContext, SETTING(SSL_PRIVATE_KEY_FILE).c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
			LogManager::getInstance()->message("Failed to load private key");
			return;
		}
	}

#ifdef _WIN32
	WIN32_FIND_DATA data;
	HANDLE hFind;

	hFind = FindFirstFile(Text::toT(SETTING(SSL_TRUSTED_CERTIFICATES_PATH) + "*.pem").c_str(), &data);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			if(SSL_CTX_load_verify_locations(clientContext, (SETTING(SSL_TRUSTED_CERTIFICATES_PATH) + Text::fromT(data.cFileName)).c_str(), NULL) != SSL_SUCCESS) {
				LogManager::getInstance()->message("Failed to load trusted certificate from " + Text::fromT(data.cFileName));
			}
		} while(FindNextFile(hFind, &data));

		FindClose(hFind);
	}
#else
#error todo
#endif
	certsLoaded = true;
}


SSLSocket* CryptoManager::getClientSocket() throw(SocketException) {
	return new SSLSocket(clientContext);
}
SSLSocket* CryptoManager::getServerSocket() throw(SocketException) {
	return new SSLSocket(serverContext);
}


void CryptoManager::decodeBZ2(const u_int8_t* is, size_t sz, string& os) throw (CryptoException) {
	bz_stream bs = { 0 };

	if(BZ2_bzDecompressInit(&bs, 0, 0) != BZ_OK)
		throw(CryptoException(STRING(DECOMPRESSION_ERROR)));

	// We assume that the files aren't compressed more than 2:1...if they are it'll work anyway,
	// but we'll have to do multiple passes...
	size_t bufsize = 2*sz;
	AutoArray<char> buf(bufsize);
	
	bs.avail_in = sz;
	bs.avail_out = bufsize;
	bs.next_in = (char*)(const_cast<u_int8_t*>(is));
	bs.next_out = buf;

	int err;

	os.clear();
	
	while((err = BZ2_bzDecompress(&bs)) == BZ_OK) { 
		if (bs.avail_in == 0 && bs.avail_out > 0) { // error: BZ_UNEXPECTED_EOF 
			BZ2_bzDecompressEnd(&bs); 
			throw CryptoException(STRING(DECOMPRESSION_ERROR)); 
		} 
		os.append(buf, bufsize-bs.avail_out); 
		bs.avail_out = bufsize; 
		bs.next_out = buf; 
	} 

	if(err == BZ_STREAM_END)
		os.append(buf, bufsize-bs.avail_out);
	
	BZ2_bzDecompressEnd(&bs);

	if(err < 0) {
		// This was a real error
		throw CryptoException(STRING(DECOMPRESSION_ERROR));	
	}
}

string CryptoManager::keySubst(const u_int8_t* aKey, size_t len, size_t n) {
	AutoArray<u_int8_t> temp(len + n * 10);
	
	size_t j=0;
	
	for(size_t i = 0; i<len; i++) {
		if(isExtra(aKey[i])) {
			temp[j++] = '/'; temp[j++] = '%'; temp[j++] = 'D';
			temp[j++] = 'C'; temp[j++] = 'N';
			switch(aKey[i]) {
			case 0: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '0'; break;
			case 5: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '5'; break;
			case 36: temp[j++] = '0'; temp[j++] = '3'; temp[j++] = '6'; break;
			case 96: temp[j++] = '0'; temp[j++] = '9'; temp[j++] = '6'; break;
			case 124: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '4'; break;
			case 126: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '6'; break;
			}
			temp[j++] = '%'; temp[j++] = '/';
		} else {
			temp[j++] = aKey[i];
		}
	}
	return string((char*)(u_int8_t*)temp, j);
}

string CryptoManager::makeKey(const string& aLock) {
	if(aLock.size() < 3)
		return Util::emptyString;

    AutoArray<u_int8_t> temp(aLock.length());
	u_int8_t v1;
	size_t extra=0;
	
	v1 = (u_int8_t)(aLock[0]^5);
	v1 = (u_int8_t)(((v1 >> 4) | (v1 << 4)) & 0xff);
	temp[0] = v1;
	
	string::size_type i;

	for(i = 1; i<aLock.length(); i++) {
		v1 = (u_int8_t)(aLock[i]^aLock[i-1]);
		v1 = (u_int8_t)(((v1 >> 4) | (v1 << 4))&0xff);
		temp[i] = v1;
		if(isExtra(temp[i]))
			extra++;
	}
	
	temp[0] = (u_int8_t)(temp[0] ^ temp[aLock.length()-1]);
	
	if(isExtra(temp[0])) {
		extra++;
	}
	
	return keySubst(temp, aLock.length(), extra);
}

void CryptoManager::decodeHuffman(const u_int8_t* is, string& os, const size_t len) throw(CryptoException) {
//	BitInputStream bis;
	int pos = 0;

	if(len < 11 || is[pos] != 'H' || is[pos+1] != 'E' || !((is[pos+2] == '3') || (is[pos+2] == '0'))) {
		throw CryptoException(STRING(DECOMPRESSION_ERROR));
	}
	pos+=5;

	int size;
	size = *(int*)&is[pos];

	pos+=4;

	dcdebug("Size: %d\n", size);
	
	unsigned short treeSize;
	treeSize = *(unsigned short*)&is[pos];

	pos+=2;

	if(len < (size_t)(11 + treeSize * 2)) 
		throw CryptoException(STRING(DECOMPRESSION_ERROR));
	Leaf** leaves = new Leaf*[treeSize];

	int i;
	for(i=0; i<treeSize; i++) {
		int chr =  is[pos++];
		int bits = is[pos++];
		leaves[i] = new Leaf(chr, bits);
	}

	BitInputStream bis(is, pos, len);

	DecNode* root = new DecNode();

	for(i=0; i<treeSize; i++) {
		DecNode* node = root;
		for(int j=0; j<leaves[i]->len; j++) {
			try {
				if(bis.get()) {
					if(node->right == NULL)
						node->right = new DecNode();

					node = node->right;
				} else {
					if(node->left == NULL)
						node->left = new DecNode();

					node = node->left;
				}
			} catch(const BitStreamException&) {
				throw CryptoException(STRING(DECOMPRESSION_ERROR));
			}
		}
		node->chr = leaves[i]->chr;
	}
	
	bis.skipToByte();
	
	// We know the size, so no need to use strange STL stuff...
	AutoArray<char> buf(size+1);

	pos = 0;
	for(i=0; i<size; i++) {
		DecNode* node = root;
		while(node->chr == -1) {
			try {
				if(bis.get()) {
					node = node->right;
				} else {
					node = node->left;
				}
			} catch(const BitStreamException&) {
				throw CryptoException(STRING(DECOMPRESSION_ERROR));
			}

			if(node == NULL) {
				for(i=0; i<treeSize; i++) {
					delete leaves[i];
				}
				
				delete[] leaves;
				delete root;

				dcdebug("Bad node found!!!\n");
				throw CryptoException(STRING(DECOMPRESSION_ERROR));
			}
		}
		buf[pos++] = (u_int8_t)node->chr;
	}
	buf[pos] = 0;
	os.assign(buf, size);

	for(i=0; i<treeSize; i++) {
		delete leaves[i];
	}
	
	delete[] leaves;
	delete root;
}
