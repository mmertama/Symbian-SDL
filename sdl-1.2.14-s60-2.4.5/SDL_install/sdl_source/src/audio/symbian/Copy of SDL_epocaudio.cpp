/*
    SDL - Simple DirectMedia Layer

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


*/

/*
    SDL_epocaudio.cpp
    Epoc based SDL audio driver implementation
    
    Markus Mertama
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_epocaudio.c,v 0.0.0.0 2001/06/19 17:19:56 hercules Exp $";
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "epoc_sdl.h"

#include <e32hal.h>


extern "C" {
#include "SDL_audio.h"
#include "SDL_error.h"
#include "SDL_audiomem.h"
#include "SDL_audio_c.h"
#include "SDL_timer.h"
#include "SDL_audiodev_c.h"
#include "SDL_sysaudio.h"
}


#include "streamplayer.h"


//#define DEBUG_AUDIO


/* Audio driver functions */

static int OpenAudio(SDL_AudioDevice *aThisdevice, SDL_AudioSpec *aSpec);
static void WaitAudio(SDL_AudioDevice *aThisdevice);
static void PlayAudio(SDL_AudioDevice *aThisdevice);
static Uint8* GetAudioBuf(SDL_AudioDevice *aThisdevice);
static void CloseAudio(SDL_AudioDevice *aThisdevice);
static void ThreadInit(SDL_AudioDevice *aThisdevice);

static int AudioAvailable();
static SDL_AudioDevice* AudioCreateDevice(int aDevindex);
static void AudioDeleteDevice(SDL_AudioDevice *aDevice);

#ifdef __WINS__
#define DODUMP
#endif

#ifdef DODUMP
NONSHARABLE_CLASS(TDump)
	{
	public:
	TInt Open();
	void Close();
	void Dump(const TDesC8& aDes);
	private:
		RFile iFile;
    	RFs iFs; 
	};
	
TInt TDump::Open()
	{
	TInt err = iFs.Connect();
	if(err == KErrNone)
		{
#ifdef __WINS__
_LIT(target, "C:\\sdlau.raw");
#else
_LIT(target, "E:\\sdlau.raw");
#endif 
		err = iFile.Replace(iFs, target, EFileWrite);
		}
	return err;
	}
void TDump::Close()
	{
	iFile.Close();
	iFs.Close();
	}
void TDump::Dump(const TDesC8& aDes)
	{
	iFile.Write(aDes);
	}
#endif

NONSHARABLE_CLASS(CAudioDoubleBuffer) : public CBase
    {
public:
    static CAudioDoubleBuffer* NewL(TInt aFill, TInt aSize);
    inline TUint8* Read() const;
    inline void Write(TUint8*& aPtr);
    inline TInt Size() const;
    inline TBool IsFree() const;
    inline void Release();
    ~CAudioDoubleBuffer();
    CAudioDoubleBuffer(TInt aSize);
private:
    const TInt iSize;
    TInt iBufsUsed;
    TUint8* iBuf[2];
    };


CAudioDoubleBuffer::~CAudioDoubleBuffer()
    {
    User::Free(iBuf);
    }

CAudioDoubleBuffer::CAudioDoubleBuffer(TInt aSize) : iSize(aSize)
    {
    }

CAudioDoubleBuffer* CAudioDoubleBuffer::NewL(TInt aFill, TInt aSize)
    {
    CAudioDoubleBuffer* b = new (ELeave) CAudioDoubleBuffer(aSize);
    CleanupStack::PushL(b);
    b->iBuf[0] = static_cast<TUint8*>(User::AllocL(aSize));
    b->iBuf[1] = static_cast<TUint8*>(User::AllocL(aSize));
    memset(b->iBuf[0], aSize, aFill);
    memset(b->iBuf[1], aSize, aFill);
    CleanupStack::Pop();
    return b;
    }

inline TUint8* CAudioDoubleBuffer::Read() const
    {
    return iBuf[iBufsUsed];
    }

inline void CAudioDoubleBuffer::Write(TUint8*& aPtr)
    {
    aPtr = iBuf[iBufsUsed];
    ++iBufsUsed;
    }

inline void CAudioDoubleBuffer::Release()
    {
    --iBufsUsed;
    }

inline TBool CAudioDoubleBuffer::IsFree() const
    {
    return EFalse; //return iBufsUsed < 2;
    }
    
inline TInt CAudioDoubleBuffer::Size() const
    {
    return iSize;
    }

NONSHARABLE_CLASS(CSimpleWait) : public CActive
    {
public:   
    static CSimpleWait* NewL();
    void Wait();
    void Release();
    ~CSimpleWait();
private:
    CSimpleWait();
    void DoCancel();
    void RunL();
private:
    TRequestStatus* iStatusPtr;
    };

CSimpleWait* CSimpleWait::NewL()
    {
    return new (ELeave) CSimpleWait();
    }

void CSimpleWait::Wait()
    {
    if(!IsActive())
        {
        iStatus = KRequestPending;
        SetActive();
        iStatusPtr = &iStatus;
        CActiveScheduler::Start();
        }
    }

void CSimpleWait::Release()
    {
    if(iStatusPtr != NULL)
        User::RequestComplete(iStatusPtr, KErrNone);
    }

CSimpleWait::CSimpleWait() : CActive(CActive::EPriorityLow)
    {
    CActiveScheduler::Add(this);
    }

CSimpleWait::~CSimpleWait()
    {
    Cancel();
    }

void CSimpleWait::DoCancel()
    {
    User::RequestComplete(iStatusPtr, KErrCancel);
    }

void CSimpleWait::RunL()
    {
    CActiveScheduler::Stop();
    }
   
/*
NONSHARABLE_CLASS(CSimpleWait) : public CTimer
	{
	public:
		void Wait(TTimeIntervalMicroSeconds32 aWait);
		static CSimpleWait* NewL();
	private:
		CSimpleWait();
		void RunL();
	};


CSimpleWait* CSimpleWait::NewL()
	{
	CSimpleWait* wait = new (ELeave) CSimpleWait();
	CleanupStack::PushL(wait);
	wait->ConstructL();
	CleanupStack::Pop();
	return wait;
	}

void CSimpleWait::Wait(TTimeIntervalMicroSeconds32 aWait)
	{
	After(aWait);
	CActiveScheduler::Start();
	}
	
CSimpleWait::CSimpleWait() : CTimer(CActive::EPriorityStandard)	
	{
	CActiveScheduler::Add(this);
	}

void CSimpleWait::RunL()
	{
	CActiveScheduler::Stop();
	}

const TInt KAudioBuffers(2);
*/	

NONSHARABLE_CLASS(CEpocAudio) : public CBase, public MStreamObs, public MStreamProvider
    {
    public:
    	static void* NewL(TInt BufferSize, TInt aFill);
    	inline static CEpocAudio& Current(SDL_AudioDevice* thisdevice);
    	
    	static void Free(SDL_AudioDevice* thisdevice);
 		
    	void Wait();
    	void Play();
    	void ThreadInitL(TAny* aDevice);
    	void Open(TInt iRate, TInt iChannels, TUint32 aType, TInt aBytes);
    	~CEpocAudio();
    	TUint8* Buffer();
    	TBool SetPause(TBool aPause);
    #ifdef DODUMP
    	void Dump(const TDesC8& aBuf) {iDump.Dump(aBuf);}
    #endif
    private:
    	CEpocAudio(/*TInt aBufferSize*/);
    	void Complete(TInt aState, TInt aError);
    	TPtrC8 Data();
    	void ConstructL(TInt aFill, TInt aBufferSize);
    private:
    	TInt iBufferSize;
    	CStreamPlayer* iPlayer;
    	TInt iBufferRate;
    	TInt iRate;
    	TInt iChannels;
    	TUint32 iType;
    	CAudioDoubleBuffer* iBuffer;
    	TBool iPause;
   // 	TInt iPosition;
  //  	TThreadId iTid;
    //	TUint8* iAudioPtr;
    //	TUint8* iBuffer;
    //	TTimeIntervalMicroSeconds iStart;
    //	TTime iStart;
    //	TInt iTune;
    	CSimpleWait* iWait;
    #ifdef DODUMP
    	TDump iDump;
    #endif
    };


inline CEpocAudio& CEpocAudio::Current(SDL_AudioDevice* thisdevice)
	{
	return *static_cast<CEpocAudio*>((void*)thisdevice->hidden);
	}
	
	
void CEpocAudio::Free(SDL_AudioDevice* thisdevice)
	{
    CEpocAudio* ea = static_cast<CEpocAudio*>((void*)thisdevice->hidden);
    if(ea)
    	{
	//	ASSERT(ea->iTid == RThread().Id());
    	delete ea;
    	thisdevice->hidden = NULL;	
   
    	CActiveScheduler* as =  CActiveScheduler::Current();
    	ASSERT(as->StackDepth() == 0);    	
    	delete as;
    	CActiveScheduler::Install(NULL);
    	}
    ASSERT(thisdevice->hidden == NULL);
	}
	
CEpocAudio::CEpocAudio(/*TInt aBufferSize*/) : iPause(ETrue)//iBufferSize(aBufferSize), iPosition(-1) 
	{
	}


void* CEpocAudio::NewL(TInt aBufferSize, TInt aFill)
	{
	CEpocAudio* eAudioLib = new (ELeave) CEpocAudio();
	CleanupStack::PushL(eAudioLib);
	eAudioLib->ConstructL(aFill, aBufferSize);
	CleanupStack::Pop();
	return eAudioLib;
	}
	
void CEpocAudio::ConstructL(TInt aFill, TInt aBufferSize)
	{
	iBuffer = CAudioDoubleBuffer::NewL(aFill, aBufferSize);
	//iBuffer = (TUint8*) User::AllocL(KAudioBuffers * iBufferSize);
	//memset(iBuffer, aFill, KAudioBuffers * iBufferSize);
//	iAudioPtr = iBuffer;
	}


TBool CEpocAudio::SetPause(TBool aPause)
	{
	if(iPause != aPause && iPlayer != NULL)
	    {
	    if(aPause)
	        iPlayer->Stop();
	    else
	        iPlayer->Start();
	    }
	iPause = aPause;
	return iPause;
	/*if(aPause && iPosition >= 0)
		{
		iPosition = -1;
		if(iPlayer != NULL)
			iPlayer->Stop();
		}
	if(!aPause && iPosition < 0)
		{
		iPosition = 0;
		if(iPlayer != NULL)
			iPlayer->Start();
		}
	return iPosition < 0;*/
	}
	
void CEpocAudio::ThreadInitL(TAny* aDevice)
	{
//	iTid = RThread().Id(); 
	CActiveScheduler* as =  new (ELeave) CActiveScheduler();
    CActiveScheduler::Install(as);
    
    EpocSdlEnv::AppendCleanupItem(TSdlCleanupItem((TSdlCleanupOperation)CloseAudio, aDevice));
   
    iWait = CSimpleWait::NewL();
   
    iPlayer = new (ELeave) CStreamPlayer(*this, *this);
    iPlayer->ConstructL();	
    iPlayer->OpenStream(iRate, iChannels, iType);
    
    #ifdef DODUMP
    User::LeaveIfError(iDump.Open());
    #endif
	}
	
	
	
CEpocAudio::~CEpocAudio()
	{
	if(iWait != NULL)
		iWait->Cancel();
	delete iWait; 
	if(iPlayer != NULL)
		iPlayer->Close();
	delete iPlayer;
	delete iBuffer;
	}
	
void CEpocAudio::Complete(TInt aState, TInt aError)
	{
	iWait->Release();
	if(aState == MStreamObs::EClose)
		{
		}
	if(iPlayer->Closed())
		return;
	switch(aError)
		{
		case KErrUnderflow:
		case KErrInUse:
			iPlayer->Start();
			break;
		case KErrAbort:
			iPlayer->Open();
		}
	}
	
TUint8* CEpocAudio::Buffer()
    {
    TUint8* ptr;
    iBuffer->Write(ptr);
    return ptr;
    /*iStart.UniversalTime();   
    return iAudioPtr;*/
    }	
	
void CEpocAudio::Play()
	{
	SetPause(EFalse);
	}

void CEpocAudio::Wait()
	{
	if(!iBuffer->IsFree())
	    iWait->Wait();
	/*
	if(iPosition >= 0 )
		{
		const TInt64 bufMs = TInt64(iBufferSize - KClip) * TInt64(1000000);
		const TInt64 specTime =  bufMs / TInt64(iRate * iChannels * 2);
		iWait->After(specTime);
		
		CActiveScheduler::Start();
		TTime end;
		end.UniversalTime();
		const TTimeIntervalMicroSeconds delta = end.MicroSecondsFrom(iStart);
	
		
	
		
		const TInt diff = specTime - delta.Int64();
		
		if(diff > 0 && diff < 200000)
			{
			User::After(diff);
			}
		
		}
	else
		{
		User::After(10000); 
		}
	*/
	}


TPtrC8 CEpocAudio::Data()
    {
    if(iPause)
        return KNullDesC8();
    iWait->Release();
    const TPtrC8 data (iBuffer->Read(), iBuffer->Size());
    iBuffer->Release();
    return data;
    /*if(iPosition < 0)
        return KNullDesC8();
    
    TPtrC8 data(iAudioPtr + iPosition, KClip);
    
#ifdef DODUMP
    iDump.Dump(data);
#endif
    
    iPosition += KClip;
    if(iPosition >= iBufferSize) 
        {
        
        
        iAudioPtr += iBufferSize;
        
        if((iAudioPtr - iBuffer) >= KAudioBuffers * iBufferSize)
            iAudioPtr = iBuffer;
        
        iPosition = -1;
        if(iWait->IsActive())
            {
            iWait->Cancel();
            CActiveScheduler::Stop();
            }
        }
    return data;*/
    }

void CEpocAudio::Open(TInt aRate, TInt aChannels, TUint32 aType, TInt aBytes)	
	{
	iRate = aRate;
	iChannels = aChannels;
	iType = aType;
    iBufferRate = iRate * iChannels * aBytes; //1/x
	}


 

/* Audio driver bootstrap functions */

AudioBootStrap EPOCAudio_bootstrap = {
	"epoc\0\0\0",
	"EPOC streaming audio\0\0\0",
	AudioAvailable,
	AudioCreateDevice
};


static SDL_AudioDevice *AudioCreateDevice(int /*aDevindex*/)
{
	SDL_AudioDevice *thisdevice;

	/* Initialize all variables that we clean on shutdown */
	thisdevice = (SDL_AudioDevice *)malloc(sizeof(SDL_AudioDevice));
	if ( thisdevice ) {
		memset(thisdevice, 0, (sizeof *thisdevice));
		thisdevice->hidden = NULL; 
	}
	if ( (thisdevice == NULL) ) {
		SDL_OutOfMemory();
		if ( thisdevice ) {
			free(thisdevice);
		}
		return(0);
	}


	/* Set the function pointers */
	thisdevice->OpenAudio = OpenAudio;
	thisdevice->WaitAudio = WaitAudio;
	thisdevice->PlayAudio = PlayAudio;
	thisdevice->GetAudioBuf = GetAudioBuf;
	thisdevice->CloseAudio = CloseAudio;
    thisdevice->ThreadInit = ThreadInit;
	thisdevice->free = AudioDeleteDevice;

	return thisdevice;
}


static void AudioDeleteDevice(SDL_AudioDevice *aDevice)
    {
	//free(device->hidden);
	free(aDevice);
    }

static int AudioAvailable()
{
	return(1); // Audio stream modules should be always there!
}


static int OpenAudio(SDL_AudioDevice *aThisdevice, SDL_AudioSpec *aSpec)
{
	SDL_TRACE("SDL:EPOC_OpenAudio");

	
	TUint32 type = KMMFFourCCCodePCM16;
	TInt bytes = 2;
	
	switch(aSpec->format)
		{
		case AUDIO_U16LSB: 
			type = KMMFFourCCCodePCMU16; 
			break;
		case AUDIO_S16LSB: 
			type = KMMFFourCCCodePCM16; 
			break;
		case AUDIO_U16MSB: 
			type = KMMFFourCCCodePCMU16B; 
			break;
		case AUDIO_S16MSB: 
			type = KMMFFourCCCodePCM16B; 
			break; 
			//8 bit not supported!
		case AUDIO_U8: 
		case AUDIO_S8:
		default:
			aSpec->format = AUDIO_S16LSB;
		};
	

	
	if(aSpec->channels > 2)
		aSpec->channels = 2;
	
	aSpec->freq = CStreamPlayer::ClosestSupportedRate(aSpec->freq);
	

	/* Allocate mixing buffer */
	const TInt buflen = aSpec->size;
    
    TRAPD(err, aThisdevice->hidden = static_cast<SDL_PrivateAudioData*>(CEpocAudio::NewL(buflen, aSpec->silence)));
    if(err != KErrNone)
        return -1;

	CEpocAudio::Current(aThisdevice).Open(aSpec->freq, aSpec->channels, type, bytes);
	
	CEpocAudio::Current(aThisdevice).SetPause(ETrue);
	

    aThisdevice->enabled = 0; /* enable only after audio engine has been initialized!*/

	/* We're ready to rock and roll. :-) */
	return(0);
}


static void CloseAudio(SDL_AudioDevice* aThisdevice)
    {
#ifdef DEBUG_AUDIO
    SDL_TRACE("Close audio\n");
#endif

	CEpocAudio::Free(aThisdevice);
	}


static void ThreadInit(SDL_AudioDevice *aThisdevice)
    {
	SDL_TRACE("SDL:EPOC_ThreadInit");
    CEpocAudio::Current(aThisdevice).ThreadInitL(aThisdevice);
    RThread().SetPriority(EPriorityMore);
    aThisdevice->enabled = 1;
    }

/* This function waits until it is possible to write a full sound buffer */
static void WaitAudio(SDL_AudioDevice* aThisdevice)
{
#ifdef DEBUG_AUDIO
    SDL_TRACE1("wait %d audio\n", CEpocAudio::AudioLib().StreamPlayer(KSfxChannel).SyncTime());
    TInt tics = User::TickCount();
#endif

	CEpocAudio::Current(aThisdevice).Wait();

#ifdef DEBUG_AUDIO
    TInt ntics =  User::TickCount() - tics;
    SDL_TRACE1("audio waited %d\n", ntics);
    SDL_TRACE1("audio at %d\n", tics);
#endif
}


 
static void PlayAudio(SDL_AudioDevice* aThisdevice)
	{
 	if(CEpocAudio::Current(aThisdevice).SetPause(SDL_GetAudioStatus() == SDL_AUDIO_PAUSED))
 		SDL_Delay(500); //hold on the busy loop
 	else
 		CEpocAudio::Current(aThisdevice).Play();

#ifdef DEBUG_AUDIO
    SDL_TRACE("buffer has audio data\n");
#endif

	
#ifdef DEBUG_AUDIO
	SDL_TRACE1("Wrote %d bytes of audio data\n", buflen);
#endif
}

static Uint8* GetAudioBuf(SDL_AudioDevice* aThisdevice)
	{
	return CEpocAudio::Current(aThisdevice).Buffer();
	}

#if ADUMP
void sos_adump(SDL_AudioDevice* thisdevice, void* data, int len)
    {
#ifdef DODUMP
    const TPtrC8 buf((TUint8*)data, len);
    CEpocAudio::Current(thisdevice).Dump(buf);
#endif
    }
#endif 




