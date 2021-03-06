// epl.ws.cpp: 定义 DLL 应用程序的导出函数。
//
#include "stdafx.h"
#include "SocksDominator.h"


DWORD APIENTRY WSLinkStart(const char *domain,
	LPFN_WS_EVENT_CALLBACK onopen,
	LPFN_WS_EVENT_CALLBACK onmessage,
	LPFN_WS_EVENT_CALLBACK onclose,
	LPFN_WS_EVENT_CALLBACK onerror) {
	USES_CONVERSION;

	SocksDominator *ws = new SocksDominator(domain, onopen, onmessage, onclose, onerror);
	ws->SetReadyState(WS_STATE_CONNECTING);
	ws->GetHandler()->connect(A2W(domain)).then([ws]() -> void {

		if (ws->GetCallback(WS_EVT_ONOPEN)) {
			ws->SetReadyState(WS_STATE_OPEN);
			ws->GetCallback(WS_EVT_ONOPEN)((DWORD)ws);

			ws->OnMsgDispatchs();
		}

	}).then([ws](pplx::task<void> t) -> void {

		try
		{
			t.get();
		}
		catch (const std::exception& ex)
		{
			if (ws->GetCallback(WS_EVT_ERROR)) {
				ws->SetReadyState(WS_STATE_CONNECTING);
				ws->GetCallback(WS_EVT_ERROR)((DWORD)ws);
			}
		}
	});
	return (DWORD)ws;
}

void APIENTRY WSDestory(DWORD e) {
	if (e) {
		delete reinterpret_cast<SocksDominator*>(e);
	}
}

void APIENTRY WSReinterpretCast(DWORD e, DWORD retval) {
	if (retval) {
		*(DWORD*)(retval + 8) = e;
	}
}

int APIENTRY WSLinkReadyState(DWORD e) {
	if (e) {
		return reinterpret_cast<SocksDominator*>(e)->GetReadyState();
	}
	return 0;
}

int APIENTRY WSLinkByteLength(DWORD e) {
	if (e) {
		auto t = reinterpret_cast<SocksDominator*>(e)->GetBody();
		if (t) {
			return t->length();
		}
	}
	return 0;
}

int APIENTRY WSLinkCopyTextBuffer(DWORD e,char *buf,int len) {
	if (e) {
		auto t = reinterpret_cast<SocksDominator*>(e)->GetBody();
		if (t) {
			if (len <= 0) {
				return t->length();
			}
			else {
				streams::stringstreambuf sBuffer;
				auto &strBody = sBuffer.collection();
				t->body().read_to_end(sBuffer).get();
				memcpy(buf, strBody.c_str(), strBody.length());
				return strBody.length();
			}
		}
	}
	return 0;
}

int APIENTRY WSLinkCopyBinaryBuffer(DWORD e, BYTE *buf, int len) {
	if (e) {
		auto t = reinterpret_cast<SocksDominator*>(e)->GetBody();
		if (t) {
			if (len <= 0) {
				return t->length();
			}
			else {
				streams::streambuf<uint8_t> f = t->body().streambuf();
				f.get_base()->scopy(buf, len);
				return len;
			}
		}
	}
	return 0;
}

int APIENTRY WSLinkCursor(DWORD e) {
	if (e) {
		auto t = reinterpret_cast<SocksDominator*>(e)->GetBody();
		if (t) {
			return (int)t->message_type();
		}
	}
	return 0;
}

int APIENTRY WSLinkSend(DWORD e, const char *buf) {
	if (e) {
		auto t = reinterpret_cast<SocksDominator*>(e);
		websocket_outgoing_message msg;
		msg.set_utf8_message(buf);
		t->GetHandler()->send(msg).then([t] (pplx::task<void> except)
		{
			try
			{
				except.get();
			}
			catch (const std::exception&)
			{
				if (t->GetCallback(WS_EVT_ERROR)) {
					t->GetCallback(WS_EVT_ERROR)((DWORD)t);
				}
			}
		});
	}
	return 0;
}

int APIENTRY WSLinkSendb(DWORD e, const BYTE *buf, int len) {
	if (e) {
		auto t = reinterpret_cast<SocksDominator*>(e);
		websocket_outgoing_message msg;
		streams::istream in;
		auto out = in.streambuf().alloc(len);
		memcpy(out, buf, len);
		msg.set_binary_message(in);
		t->GetHandler()->send(msg).then([t](pplx::task<void> except)
		{
			try
			{
				except.get();
			}
			catch (const std::exception&)
			{
				if (t->GetCallback(WS_EVT_ERROR)) {
					t->GetCallback(WS_EVT_ERROR)((DWORD)t);
				}
			}
		});
	}
	return 0;
}

void APIENTRY WSLinkClose(DWORD e) {
	if (e) {
		auto t = reinterpret_cast<SocksDominator*>(e);
		t->SetReadyState(WS_STATE_CLOSING);
		t->GetHandler()->close().then([t]() {
			t->SetReadyState(WS_STATE_CLOSE);
		});
	}
}

void CALLBACK onopen(DWORD e) {
	SocksDominator *socks = reinterpret_cast<SocksDominator*>(e);
	printf("open %d !\n", socks->GetReadyState());
}

void CALLBACK onmessage(DWORD e) {
	//SocksDominator *socks = reinterpret_cast<SocksDominator*>(e);
	//int len = WSLinkByteLength(e);
	//printf("onmessage %d!\n",len);

	//if (len > 0) {
	//	if (WSLinkSursor(e) == 0) {
	//		char *buf = new char[len + 1];
	//		WSLinkCopyTextBuffer(e,buf,len);
	//		buf[len] = 0;
	//		printf("%s \n", buf);
	//		delete[]buf;

	//		//WSLinkClose(e);
	//	}
	//	else if(WSLinkSursor(e) == 1) {
	//		WSLinkCopyBinaryBuffer()
	//	}
	//}
}

void CALLBACK onclose(DWORD e) {
	printf("onclose !\n");
}

void CALLBACK onerror(DWORD e) {
	printf("onerror !\n");
}

int main(int argc, char **argv) {

	//DWORD e = WSLinkStart("ws://121.40.165.18:8088", onopen, onmessage, onclose, onerror);
	//
	//while (WSLinkReadyState(e)!= WS_STATE_CLOSE)
	//{
	//	printf("lifed... \n");
	//	Sleep(200);
	//}
	//printf("destory! \n");
	////delete reinterpret_cast<SocksDominator*>(e);

	getchar();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}