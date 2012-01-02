// Copyright 2012 Sergio Garcia. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice, this list of
//       conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright notice, this list
//       of conditions and the following disclaimer in the documentation and/or other materials
//       provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those of the
// authors and should not be interpreted as representing official policies, either expressed
// or implied, of Sergio Garcia.
//

#include "precomp.h"

#include <iostream>
#include <fstream>
using namespace std;

// Debug
// #define LOG_REQUEST
// #define LOG_STEPS

unsigned int asp_hash(const char * str, unsigned int length)
{
	unsigned int hash = 5381;
	
	for (; length > 0; length -= 1)
	{
		hash = ((hash << 5) + hash) ^ toupper (*str++);
	}

	return hash;
}

REQUEST_NOTIFICATION_STATUS
CGinxHttpModule::OnBeginRequest(
		IN IHttpContext * pHttpContext,
		IN IHttpEventProvider * pProvider
	)
{
	UNREFERENCED_PARAMETER( pProvider );

	HRESULT hr;

	IHttpRequest * pHttpRequest = pHttpContext->GetRequest();

	if (pHttpRequest != NULL)
	{
		PCSTR strMethod = pHttpRequest->GetHttpMethod();

		if (!strMethod)
		{
			return RQ_NOTIFICATION_CONTINUE;
		}

		if (_stricmp(strMethod, "POST") != 0)
		{
			return RQ_NOTIFICATION_CONTINUE;
		}

		PCSTR strContentType = pHttpRequest->GetHeader(HttpHeaderContentType);

		if (!strContentType)
		{
			return RQ_NOTIFICATION_CONTINUE;
		}

		if (_stricmp(strContentType, "application/x-www-form-urlencoded") != 0)
		{
			return RQ_NOTIFICATION_CONTINUE;
		}

		DWORD byteCount = pHttpRequest->GetRemainingEntityBytes();

		if (byteCount < 1024)
		{
			return RQ_NOTIFICATION_CONTINUE;
		}

		DWORD cbBytesReceived = 1024;
		void * pvRequestBody = pHttpContext->AllocateRequestMemory(cbBytesReceived);

		if (NULL == pvRequestBody)
		{
			hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
			pProvider->SetErrorStatus( hr );
			return RQ_NOTIFICATION_FINISH_REQUEST;
		}

		// Retrieve the request body.
		hr = pHttpContext->GetRequest()->ReadEntityBody(pvRequestBody,cbBytesReceived,false,&cbBytesReceived,NULL);
				
		if (FAILED(hr))
		{
			if (ERROR_HANDLE_EOF != (hr  & 0x0000FFFF))
			{
				pProvider->SetErrorStatus( hr );
				return RQ_NOTIFICATION_FINISH_REQUEST;
			}
		}

		char * body = (char * ) pvRequestBody;
		body[cbBytesReceived] = 0;

		char buffer[50];
		unsigned int bi = 0;
		unsigned int previous_hash = 0;
		bool first = true;
		bool isAttack = false;

		for (int i = 0; i < cbBytesReceived; i++)
		{
			if (body[i] == '=')
			{
				buffer[bi] = 0;

				if (first)
				{
					previous_hash = asp_hash(buffer, bi);

					for (; i < cbBytesReceived; i++)
					{
						if (body[i] == '&')
						{
							break;
						}
					}

					first = false;
					bi = 0;
					
					continue;
				}
				else if (previous_hash == asp_hash(buffer, bi))
				{
					isAttack = true;
					break;
				}
				else
				{
					break;
				}
			}

			buffer[bi++] = body[i];
		}

		if (isAttack)
		{
			pHttpContext->GetResponse()->Clear();

			pHttpContext->GetResponse()->SetHeader(HttpHeaderContentType,"text/plain", (USHORT)strlen("text/plain"),TRUE);

			pHttpContext->GetResponse()->SetStatus(400, "Bad Request");

			WriteResponseMessage(pHttpContext, "A potencial hash collision attack was detected.");

			return RQ_NOTIFICATION_FINISH_REQUEST;
		}
		
	}
	
	return RQ_NOTIFICATION_CONTINUE;
}

HRESULT CGinxHttpModule::WriteResponseMessage(
	IHttpContext * pHttpContext,
	PCSTR pszResponse
)
{
	HRESULT hr;

	HTTP_DATA_CHUNK dataChunk;
	dataChunk.DataChunkType = HttpDataChunkFromMemory;
	DWORD cbSent;

	dataChunk.FromMemory.pBuffer = (PVOID) pszResponse;
	dataChunk.FromMemory.BufferLength = (USHORT) strlen(pszResponse);

	hr = pHttpContext->GetResponse()->WriteEntityChunks(&dataChunk,1,FALSE,TRUE,&cbSent);
	
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

	