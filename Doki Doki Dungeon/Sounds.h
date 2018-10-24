#pragma once
//#include "stdafx.h"
#include <windows.h>
#include "EngineCore.h"

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "./WAVFileReader.h"
#include <xaudio2.h>
#include <vector>
#include <thread>
#include <mutex> 
#pragma comment(lib,"xaudio2.lib")
//#pragma comment (lib, "winmm.lib")

using namespace std;

enum SOUNDS
{
	BUTTON,
	BACKGROUND,
	ELECTRIC,
	SLASH,
	PICKUP,
	ARROW1,
	ARROW2
};

class SoundSystems
{
private:
	//NOTE: couldn't create SourceVoice without sound, don't use this for now, may come back later if want to improve speed
	struct SourceVoices
	{
		//move SV between 2 channels boxes, don't create in the middle because it is expensive
		vector<IXAudio2SourceVoice*> idleChannels;
		vector<IXAudio2SourceVoice*> activeChannels;
	};

	IXAudio2 *audio = nullptr;
	IXAudio2MasteringVoice *masterVoice = nullptr;
	IXAudio2SubmixVoice *submixVoice = nullptr;
	vector<IXAudio2SourceVoice*> channels;
	IXAudio2SourceVoice* bgSVoice = nullptr;
	mutex bgMutex;
	mutex pathMutex;
	mutex updateChannel;
	condition_variable shutdownCondition;
	bool bgChange = false;
	//vector<thread> threads;

	SourceVoices sourceVoices;
	int maxChannels = 50;
	HRESULT hr;
	bool shutdown = false;
	bool pauseAllSounds = false;
	//HRESULT FindMediaFileCch(_Out_writes_(cchDest) WCHAR* strDestPath, _In_ int cchDest, _In_z_ LPCWSTR strFilename);

	double GetTimeWav(const WAVData* data); //in Millisecond

	HRESULT PlayWave(const wchar_t* path, float minVol = 1.0f, float lowVolTime = 0.1f, bool backGround = false);
	LPCWSTR GetSoundName(SOUNDS s);
	wstring bgPath;
	bool alreadyStartBG = false;
	float bgChangeTime = 2000; //millisecond, old BG music will still run + slower vol for a while

	unsigned int numLockMutex = 0;

	//XAUDIO2_BUFFER CreateSourceBuffer(LPCWSTR path);
	//wchar_t* GetWchar(char* c);
public:
	SoundSystems();
	~SoundSystems();

	//min Vol will slower increase to 1.0f (100%) vol
	//lowVolTime = time to increase
	void StartSoundByEnum(SOUNDS s, float minVol = 1.0f, float lowVolTime = 0.1f, bool backGround = false);

	//NOTE: use _TEXT("music.wav") or L"music.wav" for path
	//NOTE: only use this to active BG music ONCE, after that, use ChangeBGMusic
	void StartSoundByPath(LPCWSTR path, float minVol = 1.0f, float lowVolTime = 0.1f, bool backGround = false);
	void PauseAndUnPauseAllSounds();
	void DestroyAllSounds();
	// try NOT change music too fast, > 5s should be good, take time to lower Vol
	void ChangeBGMusic(LPCWSTR path);
};