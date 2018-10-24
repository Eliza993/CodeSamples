#pragma once
#include "stdafx.h"
#include "Sounds.h"
#include "./QueryPerformanceTimer.h"
#include "./LeMath.h"

LPCWSTR SoundSystems::GetSoundName(SOUNDS s)
{
	LPCWSTR name = _TEXT("");
	switch (s)
	{
	case BUTTON:
		name = _TEXT("button.wav");
		break;
	case BACKGROUND:
		name = _TEXT("bg.wav");
		break;
	case ELECTRIC:
		name = _TEXT("electric1.wav");
		break;
	case SLASH:
		name = _TEXT("slash.wav");
		break;
	case PICKUP:
		name = _TEXT("pickUp.wav");
		break;
	case ARROW1:
		name = _TEXT("Arrow1.wav");
		break;
	case ARROW2:
		name = _TEXT("Arrow2.wav");
		break;
	default:
		break;
	}
	return name;
}

//wchar_t* SoundSystems::GetWchar(char * c)
//{ 
//	int s = strlen(c) + 1;
//	wchar_t *wc = new wchar_t[s];
//	mbstowcs(wc, c, s);
//	return wc;
//}

//HRESULT SoundSystems::FindMediaFileCch(WCHAR* strDestPath, int cchDest, LPCWSTR strFilename)
//{
//	bool bFound = false;
//
//	if (!strFilename || strFilename[0] == 0 || !strDestPath || cchDest < 10)
//		return E_INVALIDARG;
//
//	// Get the exe name, and exe path
//	WCHAR strExePath[MAX_PATH] = { 0 };
//	WCHAR strExeName[MAX_PATH] = { 0 };
//	WCHAR* strLastSlash = nullptr;
//	GetModuleFileName(nullptr, strExePath, MAX_PATH);
//	strExePath[MAX_PATH - 1] = 0;
//	strLastSlash = wcsrchr(strExePath, TEXT('\\'));
//	if (strLastSlash)
//	{
//		wcscpy_s(strExeName, MAX_PATH, &strLastSlash[1]);
//
//		// Chop the exe name from the exe path
//		*strLastSlash = 0;
//
//		// Chop the .exe from the exe name
//		strLastSlash = wcsrchr(strExeName, TEXT('.'));
//		if (strLastSlash)
//			*strLastSlash = 0;
//	}
//
//	wcscpy_s(strDestPath, cchDest, strFilename);
//	if (GetFileAttributes(strDestPath) != 0xFFFFFFFF)
//		return S_OK;
//
//	// Search all parent directories starting at .\ and using strFilename as the leaf name
//	WCHAR strLeafName[MAX_PATH] = { 0 };
//	wcscpy_s(strLeafName, MAX_PATH, strFilename);
//
//	WCHAR strFullPath[MAX_PATH] = { 0 };
//	WCHAR strFullFileName[MAX_PATH] = { 0 };
//	WCHAR strSearch[MAX_PATH] = { 0 };
//	WCHAR* strFilePart = nullptr;
//
//	GetFullPathName(L".", MAX_PATH, strFullPath, &strFilePart);
//	if (!strFilePart)
//		return E_FAIL;
//
//	while (strFilePart && *strFilePart != '\0')
//	{
//		wprintf_s_s(strFullFileName, MAX_PATH, L"%s\\%s", strFullPath, strLeafName);
//		if (GetFileAttributes(strFullFileName) != 0xFFFFFFFF)
//		{
//			wcscpy_s(strDestPath, cchDest, strFullFileName);
//			bFound = true;
//			break;
//		}
//
//		wprintf_s_s(strFullFileName, MAX_PATH, L"%s\\%s\\%s", strFullPath, strExeName, strLeafName);
//		if (GetFileAttributes(strFullFileName) != 0xFFFFFFFF)
//		{
//			wcscpy_s(strDestPath, cchDest, strFullFileName);
//			bFound = true;
//			break;
//		}
//
//		wprintf_s_s(strSearch, MAX_PATH, L"%s\\..", strFullPath);
//		GetFullPathName(strSearch, MAX_PATH, strFullPath, &strFilePart);
//	}
//	if (bFound)
//		return S_OK;
//
//	// On failure, return the file as the path but also return an error code
//	wcscpy_s(strDestPath, cchDest, strFilename);
//
//	return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
//}

double SoundSystems::GetTimeWav(const WAVData* data)
{
	double result = (double)data->audioBytes / data->wfx->nAvgBytesPerSec;
	return (1000 * result);
}

//HRESULT SoundSystems::PlayWave(LPCWSTR path, float minVol, float lowVolTime, bool backGround)
HRESULT SoundSystems::PlayWave(const wchar_t* path, float minVol, float lowVolTime, bool backGround)
{
	updateChannel.lock();
	numLockMutex++;
	updateChannel.unlock();

	QueryPerformanceTimer timer; //to increase/decrease Vol
	QueryPerformanceTimer timerBG; //to decrease Vol before play new BG
	bool timerBGOn = false;
	wstring s = wstring((backGround ? MUSIC_FILEPATH : SFX_FILEPATH)) + wstring(path);
	//this 2 steps is diff. from Msdn, may need to change later

	//
	// Locate the wave file
	//

	//WCHAR strFilePath[MAX_PATH];

	//HRESULT hr = FindMediaFileCch(strFilePath, MAX_PATH, path);
	//if (FAILED(hr))
	//{
	//	wprintf(L"Failed to find media file: %s\n", path);
	//	return hr;
	//}

	//way2: no checking
	//wcscpy_s(strFilePath, MAX_PATH, path);

	//
	// Read in the wave file
	//
	std::unique_ptr<uint8_t[]> waveFile;
	DirectX::WAVData waveData;
	//if (FAILED(hr = DirectX::LoadWAVAudioFromFileEx(strFilePath, waveFile, waveData)))
	//{
	//	wprintf(L"Failed reading WAV file: %#X (%s)\n", hr, strFilePath);
	//	return hr;
	//}

	if (FAILED(hr = DirectX::LoadWAVAudioFromFileEx(s.c_str(), waveFile, waveData)))
	{
		wprintf(L"Failed reading WAV file: %#X (%s)\n", hr, s.c_str());
		return hr;
	}

	//cout << "Play " << s.c_str() << endl;

	double soundLength = GetTimeWav(&waveData);
	//cout << "Sound length: "<< soundLength << endl;	

	//
	// Play the wave using a XAudio2SourceVoice
	//

	// Create the source voice
	IXAudio2SourceVoice* sVoice = nullptr;

#if 1
	if (FAILED(hr = audio->CreateSourceVoice(&sVoice, waveData.wfx)))
	{
		wprintf(L"Error %#X creating source voice\n", hr);
		return hr;
	}

	//add new source into channels
	channels.push_back(sVoice);

#else
	//instead of creat new SV, use SV from idleChannel
	if (sSystem->sourceVoices.idleChannels.size() > 0)
	{
		sVoice = sSystem->sourceVoices.idleChannels.back();
		sSystem->sourceVoices.activeChannels.push_back(sVoice);
		sSystem->sourceVoices.idleChannels.pop_back();
	}
	else
	{
		hr = S_FALSE;
		wprintf(L"Don't have enough channel\n");
		return hr;
	}
#endif // 0

	// Submit the wave sample data using an XAUDIO2_BUFFER structure
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.pAudioData = waveData.startAudio;
	buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
	buffer.AudioBytes = waveData.audioBytes;

	if (backGround)
	{
		bgSVoice = sVoice;
		//if (waveData.loopLength > 0)
		{
			buffer.LoopBegin = waveData.loopStart;
			buffer.LoopLength = waveData.loopLength;
			//buffer.LoopCount = 1; // We'll just assume we play the loop twice			
			buffer.LoopCount = XAUDIO2_LOOP_INFINITE;			
		}
	}
	if (FAILED(hr = sVoice->SubmitSourceBuffer(&buffer)))
	{
		wprintf(L"Error %#X submitting source buffer\n", hr);
		sVoice->DestroyVoice();
		return hr;
	}

	hr = sVoice->Start(0);
	timer.StartTimer();

	//change Vol
	if (minVol != 1.0f)
	{
		sVoice->SetVolume(minVol);
	}

	// Let the sound play
	BOOL isRunning = TRUE;
	//if (backGround)
	//{
	//	bgMutex.lock();
	//}

	float ratioLength;
	float ratioVol;
	float curVol;
	bool alreadyLower = false; //use to lower Vol when Change BG
	int loopCount = 0;
	while (SUCCEEDED(hr) && isRunning && !shutdown)
	{
		//if (backGround)
		//{
		//	bgMutex.lock();
		//}
		XAUDIO2_VOICE_STATE state;
		sVoice->GetState(&state);
		isRunning = (state.BuffersQueued > 0) != 0;
		
		alreadyLower = false;
		if (!backGround || !bgChange)
		{
			//update Vol
			if (minVol != 1.0f) //NOT default
			{
				//restart timer when BG finish 1 loop
				if (timer.GetCurTimeMillisecond() >= soundLength)
				{
					timer.StartTimer();
					//cout<<"BG played "<<++loopCount<<" times"<<endl;
				}

				ratioLength = (float)(timer.GetCurTimeMillisecond() / soundLength);

				if (ratioLength > 1.0f) //cap value
				{
					ratioLength = 1.0f;
				}
				if (ratioLength < lowVolTime) //increase Vol 
				{
					ratioVol = ratioLength / lowVolTime;
					curVol = ratioVol *(1.0f - minVol) + minVol;
					//cout << "Increase Vol: " << curVol << endl;
				}
				else if (ratioLength > (1.0f - lowVolTime)) //decrease Vol
				{
					alreadyLower = true;
					ratioVol = (1.0f - ratioLength) / lowVolTime;
					curVol = ratioVol * (1.0f - minVol) + minVol;
					//cout << "Decrease Vol: " << curVol << endl;
				}
				else
				{
					curVol = 1.0f;
					//cout << "Fixed Vol: " << curVol << endl;
				}
				LeMath::Clamp(curVol, 0.0f, 1.0f);
				sVoice->SetVolume(curVol);
			}
		}

		//change BG music by recursion
		if (bgChange && backGround)
		{			
#pragma region Lower BG vol before change BG
			//timerBG didnt start yet
			if (!timerBGOn)
			{
				timerBG.StartTimer();
				timerBGOn = true;
			}

			//avoid unconsistent Vol
			if (!alreadyLower)
			{
				ratioLength = (float)(timerBG.GetCurTimeMillisecond() / bgChangeTime);

				if (ratioLength > 1.0f) //cap value
				{
					ratioLength = 1.0f;
				}

				curVol = 1.0f - ratioLength * (1.0f - minVol);
				//cout << ratioLength <<" - Decrease BG Vol: " << curVol << endl;

				LeMath::Clamp(curVol, 0.0f, 1.0f);
				sVoice->SetVolume(curVol);
			}
#pragma endregion

			//play new BG after lower Vol old BG
			if (timerBG.GetCurTimeMillisecond() >= bgChangeTime)
			{
				sVoice->Stop(0);
				sVoice->DestroyVoice();

				bgChange = false;

				PlayWave(bgPath.c_str(), minVol, lowVolTime, true);
				updateChannel.lock();
				if (!shutdown)
				{
					for (auto iter = channels.begin(); iter != channels.end(); iter++)
					{
						//cout << count << endl;
						if (*iter._Ptr == sVoice)
						{
							//cout << "Delete"<<count << endl;
							iter = channels.erase(iter);
							//channels.erase(iter);
							sVoice->DestroyVoice();
							break;
						}
						//count++;
					}
				}

				numLockMutex--;
				shutdownCondition.notify_all();
				updateChannel.unlock();
				return hr;
			}
		}

		////if (backGround && bgChange)
		//if (shutdown || (backGround && bgChange))
		//{
		//	bgChange = false;
		//	return hr;
		//}

		Sleep(10);
	}

	//cout << "Time: "<< timer.GetCurTimeMillisecond()<<endl;

#if 1
	//NOTE: this is thread, remember to lock when make change in a same container
	//cout << "Delete Sound" << endl;
	updateChannel.lock();
	//numLockMutex++;
	//if (shutdown)
	//{
	//	updateChannel.unlock();
	//}

	//int count = 0;
	if (!shutdown)
	{
		for (auto iter = channels.begin(); iter != channels.end(); iter++)
		{
			//cout << count << endl;
			if (*iter._Ptr == sVoice)
			{
				//cout << "Delete"<<count << endl;
				iter = channels.erase(iter);
				//channels.erase(iter);
				sVoice->DestroyVoice();
				break;
			}
			//count++;
		}		
	}

	numLockMutex--;
	shutdownCondition.notify_all();
	updateChannel.unlock();

#else
	updateChannel.lock();
	//instead of destroy, put it back in idle
	for (vector<IXAudio2SourceVoice*>::iterator iter = sSystem->sourceVoices.activeChannels.begin(); iter != sSystem->sourceVoices.activeChannels.end(); iter++)
	{
		if (sVoice == *iter)
		{
			sSystem->sourceVoices.activeChannels.erase(iter);
			sSystem->sourceVoices.idleChannels.push_back(sVoice);
			break;
		}
	}
	updateChannel.unlock();

#endif

	//delete[]path;
	//delete path;

	return hr;

}

SoundSystems::SoundSystems()
{
	numLockMutex = 0;
	shutdown = false;
	//create a new thread
	hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	UINT32 flags = 0;
	hr = XAudio2Create(&audio, flags);
	if (hr != S_OK)
	{
		CoUninitialize();
	}

	hr = audio->CreateMasteringVoice(&masterVoice);
	if (hr != S_OK)
	{
		if (audio != NULL)
		{
			audio->Release();
		}
		CoUninitialize();
	}

	hr = audio->CreateSubmixVoice(&submixVoice, maxChannels, 44100, 0, 0, 0, 0);
	if (hr != S_OK)
	{
		if (audio != NULL)
		{
			audio->Release();
		}
		CoUninitialize();
	}

	XAUDIO2_SEND_DESCRIPTOR sendDes = { 0, submixVoice };
	XAUDIO2_VOICE_SENDS sendList = { (UINT32)maxChannels , &sendDes };

	//WAVEFORMATEX* waveF = new WAVEFORMATEX();	
	//for (int i = 0; i < maxChannels; i++)
	//{
	//	IXAudio2SourceVoice *s;
	//	//HRESULT hr = soundSystem.audio->CreateSourceVoice(&s, waveF, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, &sendList, NULL);
	//	HRESULT hr = audio->CreateSourceVoice(&s, waveF);
	//	if (hr != S_OK)
	//	{
	//		wprintf(L"Error\n");
	//	}
	//	sourceVoices.idleChannels.push_back(s);
	//}
}

SoundSystems::~SoundSystems()
{
	/*updateChannel.try_lock();
	updateChannel.unlock();*/
	
	unique_lock<std::mutex> uLock(updateChannel);
	shutdown = true;
	DestroyAllSounds();
	shutdownCondition.wait(uLock, [&]() {return numLockMutex == 0; });

	/*while (numLockMutex != 0)
	{
		Sleep(10);
	}*/

	masterVoice->DestroyVoice(); //just be tidy

	if (audio != NULL)
	{
		audio->Release();
	}
	
	//for (int i = 0; i < threads.size(); i++)
	//{
	//	threads[i].join();
	//}

	//cv.wait(uniLock);

	//bool lock = bgMutex.try_lock();

	//bgMutex.unlock();

	//while (!bgMutex.try_lock())
	//{
	//	if (bgMutex.try_lock())
	//	{
	//		bgMutex.unlock();
	//		break;
	//	}
	//}

	//if (bgMutex.try_lock())
	//{
	//	bgMutex.unlock();
	//}
	
	//uniLock.unlock();
	//uniLock.release();

	CoUninitialize();
}

void SoundSystems::StartSoundByEnum(SOUNDS s, float minVol, float lowVolTime, bool backGround)
{
	LeMath::Clamp(minVol, 0.0f, 1.0f);
	LeMath::Clamp(lowVolTime, 0.0f, 0.5f);

	if (alreadyStartBG && backGround)
	{
		return;
	}
	else
	{
		std::thread playS(&SoundSystems::PlayWave, this, GetSoundName(s), minVol, lowVolTime, backGround);
		playS.detach();
		if (backGround)
		{
			alreadyStartBG = true;
		}
	}
}

void SoundSystems::StartSoundByPath(LPCWSTR path, float minVol, float lowVolTime, bool backGround)
{
	LeMath::Clamp(minVol, 0.0f, 1.0f);
	LeMath::Clamp(lowVolTime, 0.0f, 0.5f);

	if (alreadyStartBG && backGround)
	{
		return;
	}
	else
	{
		//dont use this way, created object will be delete after f finish and thread will have nothing to work with				
		//LPCWSTR totalPath = s.c_str();
		//const wchar_t* totalPath = const_cast<wchar_t*>(s.c_str());
		//std::thread playS(&SoundSystems::PlayWave, this, totalPath, minVol, lowVolTime, backGround);
		
		std::thread playS(&SoundSystems::PlayWave, this, path, minVol, lowVolTime, backGround);
		
		playS.detach();
		if (backGround)
		{
			alreadyStartBG = true;
		}
	}	
}

void SoundSystems::PauseAndUnPauseAllSounds()
{
	if (!pauseAllSounds)
	{
		for (unsigned int i = 0; i < channels.size(); i++)
		{
			channels[i]->Stop();
		}
		pauseAllSounds = true;
	}
	else
	{
		for (unsigned int i = 0; i < channels.size(); i++)
		{
			channels[i]->Start();
		}
		pauseAllSounds = false;
	}
}

void SoundSystems::DestroyAllSounds()
{
	/*unique_lock<std::mutex> uLock(updateChannel);
	shutdown = true;*/
	for (unsigned i = 0; i < channels.size(); i++)
	{
		channels[i]->Stop();
	}
	channels.clear();
	/*shutdownCondition.wait(uLock, [&]() {return numLockMutex == 0; });*/

	//numLockMutex = 0;
}

void SoundSystems::ChangeBGMusic(LPCWSTR path)
{
	bgPath = path;
	bgChange = true;
}
