#include <coecntrl.h>
#include <aknappui.h>
#include <aknapp.h>
#include <akndoc.h>
#include "sdltest.hrh"
#include <sdlepocapi.h>
#include <aknnotewrappers.h>
#include <eikstart.h>

const TUid KUidSdlApp={ 0xe000A7A0 };

TInt Ticks;
TInt Frames;
TInt Done;

class MExitWait
 	{
 	public:
 		virtual void DoExit(TInt aErr) = 0;
 	};   

class CExitWait : public CActive
 	{
 	public:
 		CExitWait(MExitWait& aWait);
 		void Start();
 		~CExitWait();
 	private:
 		void RunL();
 		void DoCancel();
 	private:
 		MExitWait& iWait;
 		TRequestStatus* iStatusPtr;
 	};

class CSDLWin : public CCoeControl
	{
	public:
		void ConstructL(const TRect& aRect);
		RWindow& GetWindow() const;
		void SetNoDraw();
	private:
		void Draw(const TRect& aRect) const;
	}; 	
	
class CSdlApplication : public CAknApplication
	{
private:
	// from CApaApplication
	CApaDocument* CreateDocumentL();
	TUid AppDllUid() const;
	};
	
	
class CSdlAppDocument : public CAknDocument
	{
public:
	CSdlAppDocument(CEikApplication& aApp): CAknDocument(aApp) { }
private:
	CEikAppUi* CreateAppUiL();
	};
	
		
class CSdlAppUi : public CAknAppUi, public MExitWait
    {
public:
    void ConstructL();
    ~CSdlAppUi();
private:
	void HandleCommandL(TInt aCommand);
    void StartTestL(TInt aCmd);
    void DoExit(TInt aErr);
    void HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination);
private:
	CExitWait* iWait;
	CSDLWin* iSDLWin;
	CSDL* iSdl;
	TBool iExit;
  	};	


CExitWait::CExitWait(MExitWait& aWait) : CActive(CActive::EPriorityStandard), iWait(aWait)
	{
	CActiveScheduler::Add(this);
	}
	
CExitWait::~CExitWait()
	{
	Cancel();
	}
 
void CExitWait::RunL()
	{
	if(iStatusPtr != NULL )
		iWait.DoExit(iStatus.Int());
	}
	
void CExitWait::DoCancel()
	{
	if(iStatusPtr != NULL )
		User::RequestComplete(iStatusPtr , KErrCancel);
	}
	
void CExitWait::Start()
	{
	SetActive();
	iStatusPtr = &iStatus;
	}

void CSDLWin:: ConstructL(const TRect& aRect)	
	{
	CreateWindowL();
	SetRect(aRect);
	ActivateL();
	}

	
RWindow& CSDLWin::GetWindow() const
	{
	return Window();
	}
	

void CSDLWin::Draw(const TRect& /*aRect*/) const
	{
	CWindowGc& gc = SystemGc();
	gc.SetPenStyle(CGraphicsContext::ESolidPen);
	gc.SetPenColor(KRgbGray);
	gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	gc.SetBrushColor(0xaaaaaa);
	gc.DrawRect(Rect());
	}	
	
void CSdlAppUi::ConstructL()	
	{
	BaseConstructL(CAknAppUi::EAknEnableSkin /* | ENoScreenFurniture*/ );
	 	
	iSDLWin = new (ELeave) CSDLWin;
 	iSDLWin->ConstructL(ApplicationRect());
 	
 	
  	iWait = new (ELeave) CExitWait(*this);  			
	}

void CSdlAppUi::HandleCommandL(TInt aCommand)
	{
	switch(aCommand)
		{
		case EAknCmdExit:
		case EAknSoftkeyExit:
    	case EEikCmdExit:
        	Done = 1;
     		iExit = ETrue;
  			if(iWait == NULL || !iWait->IsActive())
  				Exit();	
			break;
    	case ETestViewBuildDate:
    	    {
    	    TBuf<32> date;
    	    CSDL* sdl = CSDL::NewL();
    	    sdl->BuildDate(date);
    	    delete sdl;
    	    CAknInformationNote* info = new (ELeave) CAknInformationNote(ETrue);
    	    info->SetTimeout(CAknNoteDialog::ENoTimeout);
    	    info->ExecuteLD(date);
    	    break;
    	    }
		default:
			if(iSdl == NULL)
				StartTestL(aCommand);
		}
	}

void CSdlAppUi::StartTestL(TInt aCmd)
	{
  	TInt flags = CSDL::ENoFlags;
 	switch(aCmd)
  		{
  		case ETestGdi: flags |= CSDL::EDrawModeGdi; break; 
  		case ETestDsa: flags |= CSDL::ENoFlags; break; 
  		case ETestDsbDb: flags |= CSDL::EDrawModeDSBDoubleBuffer; break; 
  		case ETestDsbIu: flags |= CSDL::EDrawModeDSBIncrementalUpdate; break; 
  		case ETestDsbDbA: flags |= (CSDL::EDrawModeDSBDoubleBuffer | CSDL::EDrawModeDSBAsync); break; 
  		case ETestDsbIuA: flags |= (CSDL::EDrawModeDSBIncrementalUpdate | CSDL::EDrawModeDSBAsync); break; 
  		}
  	
  	iSdl = CSDL::NewL(flags);
 	
 	iSdl->SetContainerWindowL(
 					iSDLWin->GetWindow(), 
        			iEikonEnv->WsSession(),
        			*iEikonEnv->ScreenDevice());
    Done = 0;
  	iSdl->CallMainL(iWait->iStatus);    
  	iWait->Start();
	}
    
void CSdlAppUi::DoExit(TInt aErr)
	{
	if(aErr != KErrNone)
		{
		CAknErrorNote* err = new (ELeave) CAknErrorNote(ETrue);
		TBuf<64> buf;
		buf.Format(_L("SDL Error %d"), aErr);
		err->ExecuteLD(buf);
		}	
	else
		{
		CAknInformationNote* info = new (ELeave) CAknInformationNote(ETrue);
		info->SetTimeout(CAknNoteDialog::ENoTimeout);
		TBuf<64> buf;
		const TReal ticks = TReal(Ticks) / 1000.0;
		const TReal fps = TReal(Frames) / ticks;
		buf.Format(_L("Fps %f, %dms %d frames"), fps, Ticks, Frames);
		info->ExecuteLD(buf);
		}
	delete iSdl;
	iSdl = NULL;
	
	if(iExit)
		Exit();
	}
	
void CSdlAppUi::HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination)
 	{
 	if(iSdl != NULL)
 		iSdl->AppendWsEvent(aEvent);
 	CAknAppUi::HandleWsEventL(aEvent, aDestination);
 	}
 		
CSdlAppUi::~CSdlAppUi()
	{
	if(iWait != NULL)
		iWait->Cancel();
	delete iSdl;
	delete iWait;
	delete iSDLWin;
	}

CEikAppUi* CSdlAppDocument::CreateAppUiL()
    {
    return new(ELeave) CSdlAppUi();
    }	
	
TUid CSdlApplication::AppDllUid() const
    {
    return KUidSdlApp;
    }	
    

CApaDocument* CSdlApplication::CreateDocumentL()
    {
    CSdlAppDocument* document = new (ELeave) CSdlAppDocument(*this);
    return document;
    }
  
LOCAL_C CApaApplication* NewApplication()
    {
    return new CSdlApplication;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }
    

