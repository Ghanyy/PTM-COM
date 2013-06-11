#include <stdio.h>
#include <curl/curl.h>
#include <regex>
#include <iostream>
#include <string>
#include <SDKDDKVer.h>
#include <tchar.h>
#using <System.dll>
#include <windows.h>
#include <strsafe.h>


using namespace System;
using namespace System::IO::Ports;
using namespace System::Threading;
volatile bool stopBool;
void ErrorHandler(LPTSTR lpszFunction);

public ref class PortChat
{
private:
    static bool _continue;
    static SerialPort^ _serialPort;

public:
    static void Main(String^ portName, std::vector<std::string> data)
    {
        String^ message;
        StringComparer^ stringComparer = StringComparer::OrdinalIgnoreCase;
        Thread^ readThread = gcnew Thread(gcnew ThreadStart(PortChat::Read));

        // Create a new SerialPort object with default settings.
        _serialPort = gcnew SerialPort();

        // Allow the user to set the appropriate properties.
        _serialPort->PortName = portName;
        _serialPort->BaudRate = SetPortBaudRate(_serialPort->BaudRate);
        _serialPort->Parity = SetPortParity(_serialPort->Parity);
        _serialPort->DataBits = SetPortDataBits(_serialPort->DataBits);
        _serialPort->StopBits = SetPortStopBits(_serialPort->StopBits);
        _serialPort->Handshake = SetPortHandshake(_serialPort->Handshake);

        // Set the read/write timeouts
        //_serialPort->ReadTimeout = 500;
        _serialPort->WriteTimeout = 500;

        _serialPort->Open();
        _continue = true;
        //readThread->Start();
		System::String^ startBit = gcnew String("{");
		System::String^ newLineBit = gcnew String("~");
		System::String^ stopBit = gcnew String("}");
		System::String^ clr = gcnew String("]");
		int licznik=0;
		while(!stopBool)
		{
			std::vector<std::string>::iterator it;
			_serialPort->Write(startBit);Sleep(15);
			for(it=data.begin(); it!=data.end(); ++it)
			{
				if((*it).length()>21)
				{
					int off = (*it).length() - 21;
					if(licznik<off)
					{						
						System::String^ tmp = gcnew String((*it).substr(licznik,7).c_str());
						System::String^ tmp2 = gcnew String((*it).substr((licznik+7),7).c_str());
						System::String^ tmp3 = gcnew String((*it).substr((licznik+14),7).c_str());
						_serialPort->Write(tmp);Sleep(15);	
						_serialPort->Write(tmp2);Sleep(15);					
						_serialPort->Write(tmp3);Sleep(15);
					}
					else
					{
						System::String^ tmp = gcnew String((*it).substr(licznik%off,7).c_str());						
						System::String^ tmp2 = gcnew String((*it).substr(((licznik%off)+7),7).c_str());
						System::String^ tmp3 = gcnew String((*it).substr(((licznik%off)+14),7).c_str());
						_serialPort->Write(tmp);Sleep(15);
						_serialPort->Write(tmp2);Sleep(15);
						_serialPort->Write(tmp3);Sleep(15);			
					}
					_serialPort->Write(newLineBit);Sleep(15);
				}
				else
				{
					if(licznik == 0)
					{
						int off = 21-(*it).length();
						for(int i=0; i<off; i++)
						(*it).push_back(' ');
							System::String^ tmp = gcnew String((*it).substr(0,7).c_str());
							System::String^ tmp2 = gcnew String((*it).substr(7,7).c_str());
							System::String^ tmp3 = gcnew String((*it).substr(14,7).c_str());
							_serialPort->Write(tmp);Sleep(15);
							_serialPort->Write(tmp2);Sleep(15);
							_serialPort->Write(tmp3);Sleep(15);										
					}
						_serialPort->Write(newLineBit);Sleep(15);
				}
			}
			licznik++;
			Sleep(40);
			_serialPort->Write(stopBit);Sleep(15);
		}	
		_serialPort->Write(clr);Sleep(15);
        //readThread->Join();
        _serialPort->Close();Sleep(15);	
    }

    static void Read()
    {
        while (_continue)
        {
            try
            {
                String^ message = _serialPort->ReadLine();
                //Console::WriteLine(message);
            }
            catch (TimeoutException ^) { }
        }
    }

    static Int32 SetPortBaudRate(Int32 defaultPortBaudRate)
    {
        String^ baudRate;
        baudRate = defaultPortBaudRate.ToString();
        return Int32::Parse(baudRate);
    }

    static Parity SetPortParity(Parity defaultPortParity)
    {
        String^ parity;
        parity = defaultPortParity.ToString();
        return (Parity)Enum::Parse(Parity::typeid, parity);
    }

    static Int32 SetPortDataBits(Int32 defaultPortDataBits)
    {
        String^ dataBits;
        dataBits = defaultPortDataBits.ToString();
        return Int32::Parse(dataBits);
    }

    static StopBits SetPortStopBits(StopBits defaultPortStopBits)
    {
        String^ stopBits;
        stopBits = defaultPortStopBits.ToString();
        return (StopBits)Enum::Parse(StopBits::typeid, stopBits);
    }

    static Handshake SetPortHandshake(Handshake defaultPortHandshake)
    {
        String^ handshake;
        handshake = defaultPortHandshake.ToString();
        return (Handshake)Enum::Parse(Handshake::typeid, handshake);
    }

};

DWORD WINAPI MyThreadFunction(LPVOID lpParam) 
{ 
	std::vector<std::string> data = *static_cast<std::vector<std::string>*>(lpParam); 
	PortChat::Main("COM7", data);
    return 0; 
} 

void normalizacjaZnakow(std::string &readBuffer)
{
	readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("ś|Ś"), std::string("s"));
	readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("ż|Ż"), std::string("z"));
	readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("ą|Ą"), std::string("a"));
	readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("ę|Ę|è"), std::string("e"));
	readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("ł|Ł"), std::string("l"));
	readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("ź|Ź"), std::string("z"));
	readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("ć|Ć"), std::string("c"));
	readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("&oacute;"), std::string("o"));
}

static size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
	reinterpret_cast<std::string*>(userp)->append(contents, size * nmemb);
    return size * nmemb;
}

std::vector<std::string> Szukaj(std::string nazwa)
{
	CURL *curl;
	CURLcode res;
	std::string readBuffer;
	char URL[100] = "http://www.filmweb.pl/search/film?q=";
	strcat(URL, nazwa.c_str());
	char URL2[100] = "http://www.filmweb.pl/";
	std::vector<std::string> informacje;

	curl = curl_easy_init();
	if(curl)
	{
	curl_easy_setopt(curl, CURLOPT_URL, URL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	res = curl_easy_perform(curl);
	/* Check for errors */
	if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	/* always cleanup */
	curl_easy_cleanup(curl);
	
	// regular expression
	const std::tr1::regex pattern("(<h3><a class=\"hdr hdr-medium\" href=\"/)([^\"]+)");
	std::tr1::cmatch result;
	bool match = std::tr1::regex_search(readBuffer.c_str(), result, pattern);
	if(match)
	{
		std::string a = result[2];
		strcat(URL2, a.c_str());
	}
	}

	curl = curl_easy_init();
	if(curl)
	{
	curl_easy_setopt(curl, CURLOPT_URL, URL2);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	res = curl_easy_perform(curl);
	/* Check for errors */
	if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	/* always cleanup */
	curl_easy_cleanup(curl);

	// regular expresion1
	const std::tr1::regex pattern0("(<div class=filmTitle>)(.*)(?:<div class=hide><span id=filmType>)");
	std::tr1::cmatch result0;
	bool match0 = std::tr1::regex_search(readBuffer.c_str(), result0, pattern0);
	if(match0)
	{
		std::tr1::regex rx("<script type=\"text/javascript\">(.|\\n)*?</script>");
		std::string a = result0[0];
		a = std::tr1::regex_replace(a, rx, std::string(" "));
		a = std::tr1::regex_replace(a, std::tr1::regex("<(.|\\n)*?>"), std::string(" "));
		a = std::tr1::regex_replace(a, std::tr1::regex("[ ]+"), std::string(" "));
		normalizacjaZnakow(a);
		const std::tr1::regex pattern0("(?:[ ])(.*)(?:[)][ ])(.*)");
		std::tr1::cmatch result00;
		bool valid0 = std::tr1::regex_search(a.c_str(), result00, pattern0);
		if(valid0)
		{
			std::string tmp = result00[1];
			tmp.push_back(')');
			informacje.push_back(tmp);
			informacje.push_back(result00[2]);

		}
	}

	// regular expresion2
	const std::tr1::regex pattern("(<div class=filmInfo>)(.*)(</tr></table></div>)");
	std::tr1::cmatch result;
	bool match = std::tr1::regex_search(readBuffer.c_str(), result, pattern);
	if(match)
	{
		std::tr1::regex rx("<span id=filmPremierePoland style=\"display:none\">(.|\\n)*?</span> <span>");
		std::string a = result[0];
		readBuffer = std::tr1::regex_replace(a, rx, std::string(" "));
		readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("<span id=filmPremiereWorld style=\"display:none\">(.|\\n)*?</span> </a>"), std::string(" "));
		readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("<(.|\\n)*?>"), std::string(" "));
		readBuffer = std::tr1::regex_replace(readBuffer, std::tr1::regex("[ ]+"), std::string(" "));
		normalizacjaZnakow(readBuffer);
		const std::tr1::regex pattern2("(?=gatunek:)(.*)(?=produkcja:)(.*)(?=premiera:)(.*)(?=re.+yseria:)(.*)(?=scenariusz:)(.*)");
		std::tr1::cmatch result2;
		bool valid = std::tr1::regex_search(readBuffer.c_str(), result2, pattern2);
		if(valid)
		{
			informacje.push_back(result2[1]);
			informacje.push_back(result2[2]);
			informacje.push_back(result2[3]);
			informacje.push_back(result2[4]);
			informacje.push_back(result2[5]);
			informacje.push_back("------ FILMWEB ------");
		}
	}
	}
	return informacje;
}

int main(int argc, char *argv[])
{

	std::string nazwa;
	HANDLE  watek;
	  while(1)
	  {
			std::cout<<"Wpisz \"exit\" aby zakonczyc lub"<<std::endl<<"Podaj nazwe filmu: ";
			getline(std::cin, nazwa);
			std::cout<<std::endl;
			if(watek!=NULL)
			{
				stopBool=1;
				WaitForMultipleObjects(1, &watek, TRUE, INFINITE);
				CloseHandle(watek);
			}
			if(!nazwa.compare("exit"))
				break;
			nazwa = std::tr1::regex_replace(nazwa, std::tr1::regex("[ ]+"), std::string("+"));
			std::vector<std::string> *NaPlytke = new std::vector<std::string>(Szukaj(nazwa));
			stopBool=0;
			watek = CreateThread( NULL,0,MyThreadFunction,reinterpret_cast<LPVOID>(NaPlytke),0,0);
			if (watek == NULL) 
			{
			   ErrorHandler(TEXT("CreateThread"));
			   ExitProcess(3);
			}
			nazwa.erase();			
	  }
	return 0;
}

void ErrorHandler(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code.

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message.

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR) lpMsgBuf) + lstrlen((LPCTSTR) lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR) lpDisplayBuf, TEXT("Error"), MB_OK); 

    // Free error-handling buffer allocations.

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}
