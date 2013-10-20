/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Deadly Rooms of Death.
 *
 * The Initial Developer of the Original Code is
 * Caravel Software.
 * Portions created by the Initial Developer are Copyright (C) 2001, 2002 
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "Screen.h"
#include "LabelWidget.h"
#include "FrameWidget.h"
#include "OptionButtonWidget.h"
#include "FontManager.h"
#include "TextBoxWidget.h"
#include "TextBox2DWidget.h"
#include "ButtonWidget.h"
#include "ToolTipEffect.h"

#include "IncludeLib.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>

static bool m_bIsCursorVisible = true;

//Message dialog coords and dimensions.
const UINT CX_MESSAGE = 330;
const UINT CY_MESSAGE = 200;
const UINT CY_TEXTINPUT = 230;
const UINT CY_TEXTINPUT2D = 405;
const UINT CX_SPACE = 8;
const UINT CY_SPACE = 8;
const UINT CX_MESSAGE_BUTTON = 70;
const UINT CY_MESSAGE_BUTTON = CY_STANDARD_BUTTON;
const UINT CX_TEXT = CX_MESSAGE - (CX_SPACE * 2);
const UINT CY_TEXT = CY_MESSAGE - (CY_SPACE * 3) - CY_MESSAGE_BUTTON;
const int X_TEXTBOX = CX_SPACE;
const UINT CX_TEXTBOX = CX_MESSAGE - X_TEXTBOX * 2;
const int Y_MESSAGE_BUTTON = CY_TEXT + (CY_SPACE * 2);
const int X_OK1 = (CX_MESSAGE - CX_MESSAGE_BUTTON) / 2;
const int X_YES = (CX_MESSAGE - CX_SPACE) / 2 - CX_MESSAGE_BUTTON;
const int X_NO = (CX_MESSAGE + CX_SPACE) / 2;
const int X_OK2 = CX_MESSAGE/2 - CX_SPACE - CX_MESSAGE_BUTTON;
const int X_CANCEL = X_OK2 + CX_MESSAGE_BUTTON + CX_SPACE;
const int X_TEXT = CX_SPACE;
const int Y_TEXT = CY_SPACE;

static const WCHAR wszExportDir[] = {W_t('H'),W_t('o'),W_t('m'),W_t('e'),W_t('m'),W_t('a'),W_t('d'),W_t('e'),W_t(0)};

int CScreen::CX_SCREEN = 640;
int CScreen::CY_SCREEN = 480;

//
//Protected methods.
//

//*****************************************************************************
CScreen::CScreen(
//Constructor.
//
//Params:
	const UINT eSetType)			//(in)	Type of screen this is.
	: CEventHandlerWidget(WT_Screen, 0L, 0, 0, CX_SCREEN, CY_SCREEN)
	, dwLastMouseMove(0L)
	, wLastMouseX(CX_SCREEN/2)
	, wLastMouseY(CY_SCREEN/2)
   , bShowingTip(false)
	, eType(eSetType)
   , eDestScreenType(SCREENLIB::SCR_Return)
{
   this->pEffects = new CEffectList(this);

	//Set up hidden message and text input dialog widgets.
	ASSERT(g_pTheDB->IsOpen());

	//Status dialog.
	this->pStatusDialog = new CDialogWidget(0L, 0, 0, CX_MESSAGE, CY_MESSAGE);
	this->pStatusDialog->Hide();

	CLabelWidget *pLabel = new CLabelWidget(TAG_TEXT, X_TEXT, Y_TEXT,
			CX_TEXT, CY_TEXT, 0, wszEmpty);	//F_Message
	this->pStatusDialog->AddWidget(pLabel);

	//Message dialog.
	this->pMessageDialog = new CDialogWidget(0L, 0, 0, CX_MESSAGE, CY_MESSAGE);
	this->pMessageDialog->Hide();

	CButtonWidget *pButton = new CButtonWidget(TAG_YES, X_YES, Y_MESSAGE_BUTTON,
			CX_MESSAGE_BUTTON, CY_MESSAGE_BUTTON, g_pTheDB->GetMessageText(MID_Yes));
	this->pMessageDialog->AddWidget(pButton);

	pButton = new CButtonWidget(TAG_NO, X_NO, Y_MESSAGE_BUTTON,
			CX_MESSAGE_BUTTON, CY_MESSAGE_BUTTON, g_pTheDB->GetMessageText(MID_No));
	this->pMessageDialog->AddWidget(pButton);

	pButton = new CButtonWidget(TAG_OK, X_OK1, Y_MESSAGE_BUTTON,
					CX_MESSAGE_BUTTON, CY_MESSAGE_BUTTON, g_pTheDB->GetMessageText(MID_Okay));
	this->pMessageDialog->AddWidget(pButton);
	
	pLabel = new CLabelWidget(TAG_TEXT, X_TEXT, Y_TEXT,
			CX_TEXT, CY_TEXT, 0, wszEmpty);	//F_Message
	this->pMessageDialog->AddWidget(pLabel);

	CFrameWidget *pFrame = new CFrameWidget(TAG_FRAME, X_TEXT - 3, Y_TEXT - 3, 
			CX_TEXT + 6, CY_TEXT + 6, NULL);
	this->pMessageDialog->AddWidget(pFrame);
	
	//Text input dialog.
	this->pInputTextDialog =
			new CDialogWidget(0L, 0, 0, CX_MESSAGE, CY_TEXTINPUT);
	this->pInputTextDialog->Hide();

	CTextBoxWidget *pTextBox = new CTextBoxWidget(TAG_TEXTBOX,
			0, 0, CX_TEXTBOX, CY_STANDARD_TBOX);
   pTextBox->Hide();
	this->pInputTextDialog->AddWidget(pTextBox);

	CTextBox2DWidget *pTextBox2D = new CTextBox2DWidget(TAG_TEXTBOX2D,
			0, 0, CX_TEXTBOX, CY_STANDARD_TBOX2D);
   pTextBox2D->Hide();
	this->pInputTextDialog->AddWidget(pTextBox2D);

	pButton = new CButtonWidget(TAG_OK, X_OK2, Y_MESSAGE_BUTTON, 
			CX_MESSAGE_BUTTON, CY_MESSAGE_BUTTON, g_pTheDB->GetMessageText(MID_OkayNoHotkey));
	this->pInputTextDialog->AddWidget(pButton);

	pButton = new CButtonWidget(TAG_CANCEL_, X_CANCEL, Y_MESSAGE_BUTTON, 
			CX_MESSAGE_BUTTON, CY_MESSAGE_BUTTON, g_pTheDB->GetMessageText(MID_CancelNoHotkey));
	this->pInputTextDialog->AddWidget(pButton);

	pLabel = new CLabelWidget(TAG_TEXT, X_TEXT, Y_TEXT,
			CX_TEXT, CY_TEXT, 0, wszEmpty);	//F_Message
	this->pInputTextDialog->AddWidget(pLabel);

	pFrame = new CFrameWidget(TAG_FRAME, X_TEXT - 3, Y_TEXT - 3, 
			CX_TEXT + 6, CY_TEXT + 6, NULL);
	this->pInputTextDialog->AddWidget(pFrame);

	//Add them to screen.
	AddWidget(this->pMessageDialog);
	AddWidget(this->pInputTextDialog);
	AddWidget(this->pStatusDialog);

	//File selection dialog box.
	this->pFileBox = new CFileDialogWidget(0L);
	AddWidget(this->pFileBox);
	this->pFileBox->Center();
	this->pFileBox->Hide();
}

//*****************************************************************************
CScreen::~CScreen()
//Destructor.
{
	ASSERT(!this->bIsLoaded); //Unload() must be called.
   delete this->pEffects;
}

//*****************************************************************************
bool CScreen::ExportSelectFile(
//Select a file to export data to.
//
//Returns: true if export confirmed, false on cancel
//
//Params:
	const MESSAGE_ID messageID,	//(in) text prompt to display
	WSTRING &wstrExportFile,		//(in) default filename,
                                 //(out) Path + File to export to
	const FileExtensionType extensionType)	//(in) file extension
{
   wstrExportFile = filenameFilter(wstrExportFile);

	//Get default export path.
   CFiles Files;
	WSTRING wstrDefExportPath = Files.GetDatPath();
	wstrDefExportPath += wszSlash;
	wstrDefExportPath += wszExportDir;
	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
	WSTRING wstrExportPath = pCurrentPlayer ? pCurrentPlayer->Settings.GetVar(
      "ExportPath", wstrDefExportPath.c_str()) : wstrDefExportPath.c_str();

	//Ask user for export path.
   ShowCursor();
	const DWORD dwTagNo = SelectFile(wstrExportPath, wstrExportFile,
			messageID, true, extensionType);
   if (pCurrentPlayer)
   {
	   if (dwTagNo == TAG_OK)
	   {
		   //Update the export path in player settings, so next time dialog
		   //comes up it will have the same path.
		   pCurrentPlayer->Settings.SetVar("ExportPath", wstrExportPath.c_str());
		   pCurrentPlayer->Update();
	   }
	   delete pCurrentPlayer;
   }

	return dwTagNo == TAG_OK;
}

//*****************************************************************************
MESSAGE_ID CScreen::Import(
//Import a data file.
//
//Params:
	const FileExtensionType extensionType,	//(in) file extension
   DWORD& dwImportedID)                   //(out) ID of the record imported
{
    dwImportedID = 0;

   //Get default export(import) path.
	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();

   CFiles Files;
	WSTRING wstrDefExportPath = Files.GetDatPath();
	wstrDefExportPath += wszSlash;
	wstrDefExportPath += wszExportDir;
	WSTRING wstrImportPath = pCurrentPlayer ? pCurrentPlayer->Settings.GetVar("ExportPath",
         wstrDefExportPath.c_str()) : wstrDefExportPath.c_str();
	WSTRING wstrImportFile;

	//Ask player for path + file.
   ShowCursor();
	const DWORD dwTagNo = SelectFile(wstrImportPath, wstrImportFile,
			MID_ImportPath, false, extensionType);
	if (dwTagNo != TAG_OK)
	{
		delete pCurrentPlayer;
		return MID_Success;
	}

	//Update the path in player settings, so next time dialog
	//comes up it will have the same path.
   if (pCurrentPlayer)
   {
	   pCurrentPlayer->Settings.SetVar("ExportPath", wstrImportPath.c_str());
	   pCurrentPlayer->Update();
	   delete pCurrentPlayer;
   }

   switch (extensionType)
   {
      case EXT_DEMO: CDbXML::info.typeBeingImported = CImportInfo::Demo; break;
      case EXT_HOLD: CDbXML::info.typeBeingImported = CImportInfo::Hold; break;
      case EXT_PLAYER: CDbXML::info.typeBeingImported = CImportInfo::Player; break;
      default: ASSERTP(false, "Bad extension type."); return MID_Success;
   }

	SetCursor(SCREENLIB::CUR_Wait);
	ShowStatusMessage(MID_Importing);

   //Commit any changes before proceeding in case the DB gets rolled back.
   g_pTheDB->Commit();

   //Read the import file.
	MESSAGE_ID result = CDbXML::ImportXML(wstrImportFile.c_str());
   if (result == MID_OverwriteHoldPrompt || result == MID_OverwriteHoldPrompt2 ||
         result == MID_OverwritePlayerPrompt)
   {
      //Data requiring special handling was found requiring user confirmation
      //before proceeding with import.
      HideStatusMessage();
	   SetCursor();
      if (ShowYesNoMessage(result) != TAG_YES)
      {
         CDbXML::CleanUp();   //not continuing import
         return result;
      }

     	SetCursor(SCREENLIB::CUR_Wait);
	   ShowStatusMessage(MID_Importing);
      if (result == MID_OverwriteHoldPrompt || result == MID_OverwriteHoldPrompt2)
         CDbXML::info.bReplaceOldHolds = true;
      else if (result == MID_OverwritePlayerPrompt)
         CDbXML::info.bReplaceOldPlayers = true;
      result = CDbXML::ImportXML(NULL);
   }

   switch (extensionType)
   {
      case EXT_HOLD: dwImportedID = CDbXML::info.dwHoldImportedID; break;
      case EXT_PLAYER: dwImportedID = CDbXML::info.dwPlayerImportedID; break;
      case EXT_DEMO: dwImportedID = 1; break;   //value not used -- indicates a demo was in fact imported
      default: dwImportedID = 0; break;
   }

   //Display result.
   HideStatusMessage();
	SetCursor();
	ShowOkMessage(result);
	return result;
}

//*****************************************************************************
void CScreen::GoToScreen(
//Deactivate this screen and go to another one.
//
//Params:
	const UINT eNextScreen)	//(in)	Screen to be at next.
{
	SetDestScreenType(eNextScreen);
	Deactivate();
}

//*****************************************************************************
void CScreen::SetCursor(const UINT cursorType)
//Sets the image of the mouse cursor.
{
	SDL_SetCursor(g_pTheSM->GetCursor(cursorType));
}

//*****************************************************************************
bool CScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{	
	return true;
}

//*****************************************************************************
void CScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const DWORD /*dwTagNo*/,			//(in)	Widget that event applied to.
	const SDL_KeyboardEvent &Key)	//(in)	Event.
{
	switch (Key.keysym.sym)
	{
      case SDLK_RETURN:
         if (Key.keysym.mod & KMOD_ALT &&	!GetHotkeyTag(Key.keysym.sym))
            //going to next case
		case SDLK_F10:
			ToggleScreenSize();
		break;

		case SDLK_F4:
			if (Key.keysym.mod & KMOD_ALT)
				GoToScreen(SCREENLIB::SCR_None);	//boss key -- exit immediately
		break;

		case SDLK_ESCAPE:
			Deactivate();
		break;

      default: break;
	}
}

//*****************************************************************************
void CScreen::OnMouseDown(
//When clicking a mouse button, make mouse cursor appear.
//
//Params:
	const DWORD /*dwTagNo*/, const SDL_MouseButtonEvent &Button)
{
	ShowCursor();
   this->pEffects->RemoveEffectsOfType(EFFECTLIB::ETOOLTIP);
	this->bShowingTip = false;

   this->dwLastMouseMove = SDL_GetTicks();
	this->wLastMouseX = Button.x;
	this->wLastMouseY = Button.y;
}

//*****************************************************************************
bool CScreen::OnQuit()
//Called when SDL_QUIT event is received.
//Returns whether Quit action was confirmed.
{
   static bool bQuitDialogActive;
   if (bQuitDialogActive) return true; //to stop a recursive call from CDialogWidget::OnQuit().
   bQuitDialogActive = true;
   const DWORD dwTagNo = ShowYesNoMessage(MID_ReallyQuit);
   bQuitDialogActive = false;
	if (dwTagNo == TAG_YES || dwTagNo == TAG_QUIT)
	{
		GoToScreen(SCREENLIB::SCR_None);
		return true;
	}
	return false;
}

//*****************************************************************************
void CScreen::OnMouseMotion(
//When moving the mouse substantially, make mouse cursor appear and erase tooltips.
//
//Params:
	const DWORD /*dwTagNo*/,
	const SDL_MouseMotionEvent &MotionEvent)
{
	static const UINT MIN_MOUSE_MOVE = 3;
	if (abs(MotionEvent.xrel) >= MIN_MOUSE_MOVE ||
			abs(MotionEvent.yrel) >= MIN_MOUSE_MOVE)	//protect against jiggles
   {
		ShowCursor();
      RemoveToolTip();
   }

	this->dwLastMouseMove = SDL_GetTicks();
	this->wLastMouseX = MotionEvent.x;
	this->wLastMouseY = MotionEvent.y;
}

//*****************************************************************************
void CScreen::OnBetweenEvents()
//If mouse hasn't moved for a bit, flag screen as ready to display a tool tip.
//When mouse is inactive for a long time, hide mouse cursor.
{
	//Show tool tip after mouse inactivity, if applicable.
	const Uint32 dwNow = SDL_GetTicks();
	const bool bTimeToShowTip = dwNow - this->dwLastMouseMove > 1000; //1 second
	this->bShowTip = (bTimeToShowTip && !this->bShowingTip);

	if (dwNow - this->dwLastMouseMove > 6000)	//6 seconds
		HideCursor();

   //Draw effects onto screen.
   this->pEffects->DrawEffects();
}

//*****************************************************************************
void CScreen::HideCursor()
//Hide the mouse cursor.
{
	if (m_bIsCursorVisible)
	{
		SDL_ShowCursor(SDL_DISABLE);
		m_bIsCursorVisible = false;
	}
}

//*****************************************************************************
void CScreen::ShowCursor()
//Show the mouse cursor.
{
	if (!m_bIsCursorVisible)
	{
		SDL_ShowCursor(SDL_ENABLE);
		if (IsFullScreen())
			//Keep mouse where it was when it was hidden.
			SDL_WarpMouse(this->wLastMouseX,this->wLastMouseY);
		m_bIsCursorVisible = true;
	}
}

//*****************************************************************************
bool CScreen::IsCursorVisible() const {return m_bIsCursorVisible;}

//*****************************************************************************
//Returns: whether display mode is full screen
bool CScreen::IsFullScreen() const
{
	SDL_Surface *pScreenSurface = GetDestSurface();
	return (pScreenSurface->flags & SDL_FULLSCREEN) != 0;
}

//*****************************************************************************
void CScreen::SetFullScreen(
//Sets this screen and all future screens to display fullscreen/windowed.
//
//Params:
	const bool bSetFull) //(in) true = Fullscreen; false = windowed
{
	if (!IsLocked() && (bSetFull != IsFullScreen()))
	{
		SDL_Surface *pScreenSurface = 
				SDL_SetVideoMode(640, 480, 24, bSetFull ? SDL_FULLSCREEN : 0);
      if (pScreenSurface)
		   SetWidgetScreenSurface(pScreenSurface);
		Paint();
	}
}

//*****************************************************************************
DWORD CScreen::SelectFile(
//Displays a dialog box with a file list box.
//Hitting OK will set fileName to the selected file.
//Hitting Cancel will leave it unchanged.
//
//Return: tag # of button clicked
//
//Params:
	WSTRING &filePath,	//(in/out) File path; new path on OK.
	WSTRING &fileName,	//(in/out) Default filename; File path + filename on OK.
	const MESSAGE_ID messagePromptID,	//(in)
	const bool bWrite,	//(in) whether the file being selected is being written to
	const FileExtensionType extensionType)	//(in) file extension (default=EXT_ALL)
{
	this->pFileBox->SetPrompt(messagePromptID);
	this->pFileBox->SetExtension(extensionType);

	//Select current choice.
	this->pFileBox->SetDirectory(filePath.c_str());
	this->pFileBox->SetWriting(bWrite);
	if (bWrite && fileName.length())
		this->pFileBox->SetFilename(fileName.c_str());
	else
		this->pFileBox->SelectFile();

	this->pFileBox->Show();
	bool bConfirmed;
	DWORD dwTagNo;
	do {
		bConfirmed = true;
		dwTagNo = this->pFileBox->Display();
		//Get selected value.
		if (dwTagNo == TAG_OK)
		{
			filePath = this->pFileBox->GetSelectedDirectory();
			fileName = this->pFileBox->GetSelectedFileName();
			if (bWrite && CFiles::DoesFileExist(fileName.c_str()))
				if (ShowYesNoMessage(MID_OverwriteFilePrompt) != TAG_YES)
					bConfirmed = false;
		}
	} while (!bConfirmed);
	this->pFileBox->Hide();
	Paint();

	return dwTagNo;
}

//*****************************************************************************
void CScreen::Paint(
//Overridable method to paint the screen.  
//
//Params:
	bool bUpdateRect)					//(in)	If true (default) and destination
										//		surface is the screen, the screen
										//		will be immediately updated in
										//		the widget's rect.
//
//As a placeholder, it justs draws a black screen to show something upon
//arrival to the screen.
{
	SDL_Surface *pScreenSurface = GetDestSurface();
	SDL_Rect rect = {0, 0, pScreenSurface->w, pScreenSurface->h};
	static const SURFACECOLOR Black = {0, 0, 0};
	DrawFilledRect(rect, Black);

	PaintChildren();

   this->pEffects->DrawEffects(false, pScreenSurface);

	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CScreen::RemoveToolTip()
{
   this->pEffects->RemoveEffectsOfType(EFFECTLIB::ETOOLTIP);
   this->bShowingTip = false;
}

//*****************************************************************************
void CScreen::RequestToolTip(const MESSAGE_ID messageID)
//If this->bShowTip indicates that the app is in a state ready to display
//a tooltip, then remove any existing tooltips and display this tooltip message.
{
 	if (this->bShowTip)
	{
      this->pEffects->RemoveEffectsOfType(EFFECTLIB::ETOOLTIP);
      CEffect *pEffect = new CToolTipEffect(this, CCoord(this->wLastMouseX,
            this->wLastMouseY), g_pTheDB->GetMessageText(messageID));
      ASSERT(pEffect);
	   this->pEffects->AddEffect(pEffect);

      this->bShowingTip = true;
	}
}

//*****************************************************************************
DWORD CScreen::ShowOkMessage(
//Display a message in a dialog.  Dialog has an okay button and execution
//waits for the button to be pushed before returning.
//
//Params:
	const MESSAGE_ID dwMessageID)	//(in)	Indicates message to display.
//
//Returns:
//TAG_OK or TAG_QUIT.
{
	//Show just the "Ok" button.
	CWidget *pButton = this->pMessageDialog->GetWidget(TAG_OK);
	pButton->Show();
   pButton = this->pMessageDialog->GetWidget(TAG_YES);
	pButton->Hide(false);
	pButton = this->pMessageDialog->GetWidget(TAG_NO);
	pButton->Hide(false);

	//Activate the dialog widget.
	return ShowMessage(dwMessageID);
}

//*****************************************************************************
DWORD CScreen::ShowYesNoMessage(
//Display a message in a dialog.  Dialog has a yes and no button and execution
//waits for a button to be pushed before returning.
//
//Params:
	const MESSAGE_ID dwMessageID)	//(in)	Indicates message to display.
//
//Returns:
//TAG_YES (user pressed yes button), TAG_NO (user pressed no button),
//or TAG_QUIT (SDL_Quit was received).
{
	//Show just the "Yes" and "No" buttons.
	CWidget *pButton = this->pMessageDialog->GetWidget(TAG_OK);
	pButton->Hide(false);
	pButton = this->pMessageDialog->GetWidget(TAG_YES);
	pButton->Show();
	pButton = this->pMessageDialog->GetWidget(TAG_NO);
	pButton->Show();

	//Activate the dialog widget.
	return ShowMessage(dwMessageID);
}

//*****************************************************************************
DWORD CScreen::ShowTextInputMessage(
//Display a message in a dialog.  Dialog has a yes and no button and execution
//waits for a button to be pushed before returning.
//
//Params:
	const MESSAGE_ID dwMessageID,	//(in)	Indicates message to display.
	WSTRING &wstrUserInput,		//(in/out)  Default text/text entered by user.
   const bool bMultiLineText, //(in)      if true, show 2D text box, else single-line text box
	const bool bMustEnterText)	//(in)		if true, OK button is disabled
										//				when text box is empty
                              //          (default = true)
{
	//Load text for message.
	const WCHAR *pwczMessage = g_pTheDB->GetMessageText(dwMessageID);
	if (!pwczMessage) pwczMessage = g_pTheDB->GetMessageText(MID_MessageLoadError);

	return ShowTextInputMessage(pwczMessage, wstrUserInput, bMultiLineText, bMustEnterText);
}

//*****************************************************************************
DWORD CScreen::ShowTextInputMessage(
//Display a message in a dialog.  Dialog has an OK and Cancel button and execution
//waits for a button to be pushed before returning.
//
//Params:
	const WCHAR* pwczMessage,	//(in)		Indicates message to display.
	WSTRING &wstrUserInput,		//(in/out)  Default text/text entered by user.
   const bool bMultiLineText, //(in)      if true, show 2D text box, else single-line text box
	const bool bMustEnterText)	//(in)		if true, OK button is disabled
										//				when text box is empty
                              //          (default = true)
//
//Returns:
//TAG_OK or TAG_CANCEL (button pressed).
//or TAG_QUIT (SDL_Quit was received).
{
	CLabelWidget *pText = static_cast<CLabelWidget *>(
			this->pInputTextDialog->GetWidget(TAG_TEXT));
	pText->SetText(pwczMessage);

	//Resize label text for text height.
	UINT wIgnored;
	SDL_Rect rect;
	pText->GetRect(rect);
	UINT wTextHeight;
   g_pTheFM->GetTextRectHeight(FONTLIB::F_Message,
			pwczMessage, rect.w, wIgnored, wTextHeight);
	pText->Resize(rect.w, wTextHeight);

   //Resize frame around text prompt to correct size.
	const int dy = wTextHeight - rect.h;
	CWidget *pWidget = this->pInputTextDialog->GetWidget(TAG_FRAME);
	pWidget->GetRect(rect);
	pWidget->Resize(rect.w, rect.h + dy);

   //Select single- or multi-line text entry.
   CTextBoxWidget *pTextBox;
   DWORD dwTextBoxTag;
   UINT CY_TEXTBOX;
   if (bMultiLineText)
   {
	   CTextBox2DWidget *pTextBox2D = DYN_CAST(CTextBox2DWidget*, CWidget*,
		   	this->pInputTextDialog->GetWidget(TAG_TEXTBOX2D));
      pTextBox = static_cast<CTextBoxWidget*>(pTextBox2D);
      dwTextBoxTag = TAG_TEXTBOX2D;
      CY_TEXTBOX = CY_STANDARD_TBOX2D;
   } else {
	   CTextBoxWidget *pTextBox1D = DYN_CAST(CTextBoxWidget*, CWidget*,
			   this->pInputTextDialog->GetWidget(TAG_TEXTBOX));
      pTextBox = pTextBox1D;
      dwTextBoxTag = TAG_TEXTBOX;
      CY_TEXTBOX = CY_STANDARD_TBOX;
   }
	pTextBox->Move(X_TEXTBOX, wTextHeight + CY_SPACE * 2);
	pTextBox->SetText(wstrUserInput.c_str());
   pTextBox->Show();

	this->pInputTextDialog->SetEnterText(bMustEnterText ? dwTextBoxTag : 0L);

	//Resize the rest of the dialog.
	const int yButtons = wTextHeight + CY_TEXTBOX + (CY_SPACE * 3);
	pWidget = this->pInputTextDialog->GetWidget(TAG_OK);
	pWidget->Move(X_OK2, yButtons);
	pWidget->Show();

	pWidget = this->pInputTextDialog->GetWidget(TAG_CANCEL_);
	pWidget->Move(X_CANCEL, yButtons);

   //Resize dialog to correct height.
	this->pInputTextDialog->GetRect(rect);
	this->pInputTextDialog->Resize(rect.w, yButtons + pWidget->GetH() + CY_SPACE);

	//Center the dialog on the screen.
	this->pInputTextDialog->Center();

	//Activate the dialog widget.
	bool bWasCursorVisible = IsCursorVisible();
	ShowCursor();
	const DWORD dwRet = this->pInputTextDialog->Display();
	if (!bWasCursorVisible) HideCursor();
   pTextBox->Hide();
	
	//Repaint the screen to fix area underneath dialog.
	Paint();

	//Return text entered and tag.
	if (dwRet == TAG_OK)
		wstrUserInput = pTextBox->GetText();
	return dwRet;
}

//*****************************************************************************
void CScreen::UpdateRect() const
//Updates entire dest (screen) surface.
{
   if (IsLocked()) return; //never call SDL_UpdateRect when surface is locked

   SDL_UpdateRect(GetDestSurface(), 0, 0, 0, 0);
}

//*****************************************************************************
void CScreen::UpdateRect(const SDL_Rect &rect) const
//Updates dest surface in this absolute screen rect.
{
   if (IsLocked()) return; //never call SDL_UpdateRect when surface is locked

   //Bounds checking.
	ASSERT(rect.x >= 0);
	ASSERT(rect.y >= 0);
	ASSERT(rect.x + rect.w <= CX_SCREEN);
	ASSERT(rect.y + rect.h <= CY_SCREEN);
   //We want the asserts to catch everything during testing,
   //but we'll put an actual check in code to prevent crashing, just in case.
   if (rect.x >= 0 && rect.y >= 0 && rect.x + rect.w <= CX_SCREEN &&
         rect.y + rect.h <= CY_SCREEN)
      SDL_UpdateRect(GetDestSurface(), rect.x, rect.y, rect.w, rect.h);
}

//*****************************************************************************
void CScreen::ToggleScreenSize()
//Toggles between window and fullscreen mode.
//Updates player settings in DB, if current player exists.
{
	SetFullScreen(!IsFullScreen());

	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
   if (pCurrentPlayer)
   {
	   pCurrentPlayer->Settings.SetVar("Fullscreen", IsFullScreen());
	   pCurrentPlayer->Update();
	   delete pCurrentPlayer;
   }

   //On some systems, must refresh the screen.
   Paint();
}

//
//Private methods.
//

//*****************************************************************************
DWORD CScreen::ShowMessage(
//Display a message in a dialog.  Execution waits for a button to be pushed 
//before returning.
//
//Params:
	const MESSAGE_ID dwMessageID)	//(in)	Indicates message to display.
//
//Returns:
//TAG_YES (user pressed yes button), TAG_NO (user pressed no button),
//or TAG_QUIT (SDL_Quit was received).
{
	//Load text for message.
	const WCHAR *pwczMessage = g_pTheDB->GetMessageText(dwMessageID);
	if (!pwczMessage) 
	{
		WSTRING wstrErr;
		AsciiToUnicode("Error: Could not retrieve message.", wstrErr); 
		pwczMessage = wstrErr.c_str();
	}
	CLabelWidget *pText = DYN_CAST(CLabelWidget*, CWidget*,
			this->pMessageDialog->GetWidget(TAG_TEXT));
	pText->SetText(pwczMessage);
	
	//Resize label text for text height.
	SDL_Rect rect;
	pText->GetRect(rect);
	UINT wIgnored;
   UINT wTextHeight;
   g_pTheFM->GetTextRectHeight(FONTLIB::F_Message,
			pwczMessage, rect.w, wIgnored, wTextHeight);
	pText->Resize(rect.w, wTextHeight);
	const int dy = (int) wTextHeight - (int) rect.h;

	//Resize the rest of the dialog.
	int yButtons = wTextHeight + (CY_SPACE * 2);
	CWidget *pWidget = this->pMessageDialog->GetWidget(TAG_YES);
	pWidget->Move(X_YES, yButtons);

	pWidget = this->pMessageDialog->GetWidget(TAG_NO);
	pWidget->Move(X_NO, yButtons);

	pWidget = this->pMessageDialog->GetWidget(TAG_OK);
	pWidget->Move(X_OK1, yButtons);

	pWidget = this->pMessageDialog->GetWidget(TAG_FRAME);
	pWidget->GetRect(rect);
	pWidget->Resize(rect.w, rect.h + dy);

	this->pMessageDialog->GetRect(rect);
	this->pMessageDialog->Resize(rect.w, rect.h + dy);

	//Center the dialog on the screen.
	this->pMessageDialog->Center();
    ASSERT(this->pMessageDialog->IsInsideOfParent()); //If this fires, probably the dialog text is too long.

	//Activate the dialog widget.
	bool bWasCursorVisible = IsCursorVisible();
	ShowCursor();
	DWORD dwRet = this->pMessageDialog->Display();
	if (!bWasCursorVisible) HideCursor();

	//Repaint the screen to fix area underneath dialog.
	Paint();

	return dwRet;
}

//*****************************************************************************
void CScreen::ShowStatusMessage(
//Display a message in a dialog.  Execution continues.
//Call HideStatusMessage() to make dialog box disappear.
//
//Params:
	const MESSAGE_ID dwMessageID)	//(in)	Indicates message to display.
//
//Returns:
//TAG_YES (user pressed yes button), TAG_NO (user pressed no button),
//or TAG_QUIT (SDL_Quit was received).
{
	//Load text for message.
	const WCHAR *pwczMessage = g_pTheDB->GetMessageText(dwMessageID);
	if (!pwczMessage) 
	{
		WSTRING wstrErr;
		AsciiToUnicode("Error: Could not retrieve message.", wstrErr);
		pwczMessage = wstrErr.c_str();
	}
	CLabelWidget *pText = DYN_CAST(CLabelWidget*, CWidget*,
			this->pStatusDialog->GetWidget(TAG_TEXT));
	pText->SetText(pwczMessage);
	
	//Resize label text for text height.
	SDL_Rect rect;
	pText->GetRect(rect);
	UINT wIgnored, wTextHeight;
   g_pTheFM->GetTextRectHeight(FONTLIB::F_Message,
			pwczMessage, rect.w, wIgnored, wTextHeight);
	pText->Resize(rect.w, wTextHeight);

	//Resize the dialog.
	this->pStatusDialog->GetRect(rect);
	this->pStatusDialog->Resize(rect.w, wTextHeight + CY_SPACE * 2);

	//Center the dialog on the screen.
	this->pStatusDialog->Center();

	//Activate the dialog widget.
	ShowCursor();
	this->pStatusDialog->Show();
	this->pStatusDialog->Paint();
}

//*****************************************************************************
void CScreen::HideStatusMessage()
//Hide the status dialog.
{
	this->pStatusDialog->Hide();

	//Repaint the screen to fix area underneath dialog.
	Paint();
}

// $Log: Screen.cpp,v $
// Revision 1.36  2005/03/15 22:14:04  mrimer
// Alt-F4 now exits instantly.
//
// Revision 1.35  2003/10/20 17:48:19  erikh2000
// Removed LOGCONTEXT macros.
//
// Revision 1.34  2003/10/07 21:11:31  erikh2000
// Added logging context for more important tasks.
//
// Revision 1.33  2003/10/06 02:42:51  erikh2000
// Added description to assertions.
//
// Revision 1.32  2003/10/01 15:57:30  mrimer
// Fixed UpdateRect() method inheritance from CWidget to CScreen.
//
// Revision 1.31  2003/09/22 16:57:09  mrimer
// Added check to avoid calling SDL_UpdateRect with invalid screen regions.
//
// Revision 1.30  2003/09/16 01:25:00  mrimer
// Now repaints the screen when switching to windowed mode.
//
// Revision 1.29  2003/08/27 17:39:44  mrimer
// FrontEndLib simplification.
//
// Revision 1.28  2003/08/16 03:58:18  mrimer
// Added RemoveToolTip().
//
// Revision 1.27  2003/08/16 00:47:40  mrimer
// Added screen effect and tooltip support.
//
// Revision 1.26  2003/08/09 20:08:10  mrimer
// Modified Import() to set the dwImportedID to 0 on unsuccessful import.
//
// Revision 1.25  2003/08/01 17:22:31  mrimer
// Fixed bug: memory not being freed when import is cancelled.
//
// Revision 1.24  2003/07/29 03:58:25  mrimer
// Fixed bug: invisible cursor on import/export dialog.
//
// Revision 1.23  2003/07/22 18:42:19  mrimer
// Changed reinterpret_casts to dynamic_casts.
//
// Revision 1.22  2003/07/19 02:11:00  mrimer
// Added optional use of multi-line text editor dialog.
//
// Revision 1.21  2003/07/12 22:13:59  mrimer
// Clicking "X" when a dialog box is active now brings up quit dialog instead of just closing the dialog.
//
// Revision 1.20  2003/07/12 20:30:58  mrimer
// Added a call to Paint() when toggling to full screen.
// Updated widget load/unload methods.
//
// Revision 1.19  2003/07/12 01:11:32  mrimer
// Fixed bug: app doesn't close on TAG_QUIT commands.
// Revised screen/cursor naming in FrontEndLib.
//
// Revision 1.18  2003/07/12 00:17:48  mrimer
// Revised Import parameters to return the ID assigned to the imported data record, so that it can be selected on successful import.
//
// Revision 1.17  2003/07/10 15:55:01  mrimer
// WCHAR string port fixes (committed on behalf of Gerry JJ).
//
// Revision 1.16  2003/07/09 21:04:55  mrimer
// Made import logic more robust.  Optimized import when giving user prompts.
//
// Revision 1.15  2003/07/08 15:58:31  mrimer
// Fixed a bug in the code just checked in.
//
// Revision 1.14  2003/07/07 23:40:49  mrimer
// Added logic to handle more cases in import routine.
//
// Revision 1.13  2003/06/30 19:28:25  mrimer
// Tweaking.
//
// Revision 1.12  2003/06/26 17:20:03  mrimer
// Removed player dependency in ExportSelectFile() too.
//
// Revision 1.11  2003/06/26 16:57:47  mrimer
// Fixed bugs: import and toggling screen assume local player exists.
//
// Revision 1.10  2003/06/20 20:45:51  mrimer
// Linux porting (committed on behalf of Gerry JJ).
//
// Revision 1.9  2003/06/17 23:12:36  mrimer
// Changed ShowTextInputMessage() to require text entry by default.
//
// Revision 1.8  2003/06/16 18:47:32  mrimer
// Hitting ESC on Exit? prompt now returns to the game.
//
// Revision 1.7  2003/06/12 21:45:58  mrimer
// Now expect default export file name to be passed in.
//
// Revision 1.6  2003/06/10 01:21:49  mrimer
// Revised some code.
//
// Revision 1.5  2003/06/06 18:08:42  mrimer
// Added some SDL checks.  Fixed some assertion bugs.
//
// Revision 1.4  2003/05/28 23:01:17  erikh2000
// Calling GetDatPath() differently.
//
// Revision 1.3  2003/05/25 22:44:36  erikh2000
// Revised #includes:
// 1. Consistent use of <> instead of "" for backendlib and frontendlib.
// 2. MID constants separated from MessageIDs.h to reduce dependencies during builds.
//
// Revision 1.2  2003/05/24 03:03:40  mrimer
// Modified some remaining DROD-specific code.
//
// Revision 1.1  2003/05/22 21:49:30  mrimer
// Initial check-in (files taken from DROD directory and modified as needed).
//
// Revision 1.66  2003/05/08 23:26:14  erikh2000
// Revised to support v1.6 format.
//
// Revision 1.65  2003/05/08 22:04:42  mrimer
// Replaced local instances of CDb with pointer to global instance.
//
// Revision 1.64  2003/05/03 23:32:22  mrimer
// Added optional enforcement of entering some text on a text input dialog.
//
// Revision 1.63  2003/04/29 11:09:27  mrimer
// Added import/export file extension specifications.
//
// Revision 1.62  2003/04/28 22:57:19  mrimer
// Added ExportSelectFile().
//
// Revision 1.61  2003/04/28 22:25:07  mrimer
// Added confirmation on file overwrite.
//
// Revision 1.60  2003/04/26 17:14:47  mrimer
// Added a extra parameter to SelectFile().
//
// Revision 1.59  2003/04/15 14:59:36  mrimer
// Added support for multiple cursor graphics.
// Set hourglass cursor during data import.
//
// Revision 1.58  2003/04/08 13:08:28  schik
// Added Visual Studio.NET project files
// Made minor modifications to make it compile with .NET
//
// Revision 1.57  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.56  2003/02/17 03:28:09  erikh2000
// Removed L() macro.
//
// Revision 1.55  2003/02/17 00:51:06  erikh2000
// Removed L" string literals.
//
// Revision 1.54  2003/02/16 20:32:19  erikh2000
// Changed wstring to WSTRING.
//
// Revision 1.53  2003/01/08 00:56:00  mrimer
// Changed SelectLevelID() to return a bool on OK.
//
// Revision 1.52  2002/12/22 02:30:00  mrimer
// Added Import, SelectFile, SelectLevelID interfaces.  Added ShowStatusMessage().
//
// Revision 1.51  2002/11/15 02:26:01  mrimer
// Added a ShowTextInputMessage(const MESSAGE_ID) method.  Made some parameters const.
//
// Revision 1.50  2002/10/22 21:24:04  mrimer
// Added OnMouseDown(), which makes cursor appear.
// Removed unneeded includes.
//
// Revision 1.49  2002/10/21 20:26:48  mrimer
// When making mouse reappear, only warp to last position in full screen mode.
//
// Revision 1.48  2002/10/18 23:02:36  mrimer
// Fixed problem with mouse cursor disappearing with small movements.
//
// Revision 1.47  2002/10/17 01:51:30  erikh2000
// Fixed a problem with ineffectual call to SDL_WarpMouse().
//
// Revision 1.46  2002/10/16 23:03:11  mrimer
// Added wLastMouseX/Y to preserve mouse cursor position when hiding.
// Made wait time longer before hiding cursor, and movement offset minimum smaller for making it reappear.
//
// Revision 1.45  2002/10/14 17:20:37  mrimer
// Fixed sufficient cursor movement detection bug.
//
// Revision 1.44  2002/10/11 17:36:13  mrimer
// Modified OnMouseMotion() to only show cursor if substantial mouse movement is made.
//
// Revision 1.43  2002/10/10 01:00:54  erikh2000
// Removed dead code.
//
// Revision 1.42  2002/10/04 18:01:26  mrimer
// Refactored quit logic into OnQuit().
//
// Revision 1.41  2002/10/03 22:43:51  mrimer
// Added code to show mouse cursor when moved, then hide again after some non-activity.
//
// Revision 1.40  2002/09/24 21:33:10  mrimer
// Removed superfluous, erroneous hotkey mappings.
//
// Revision 1.39  2002/09/04 20:40:15  mrimer
// Moved UpdateRect(rect) form CWidget to CScreen.
//
// Revision 1.38  2002/08/28 21:39:19  erikh2000
// Moved cursor creation code out of CScreen and into CScreenManager.  A cursor is now created just one time.
//
// Revision 1.37  2002/07/22 17:36:15  mrimer
// Restricted alt-Enter to only change screen size when no Enter hotkey is active.
//
// Revision 1.36  2002/07/19 20:41:42  mrimer
// Re-added ENTER hotkey for Okay button w/o 'O' hotkey.
//
// Revision 1.35  2002/07/17 21:18:51  mrimer
// Fixed widgets appearing at wrong times with dialog boxes.
//
// Revision 1.34  2002/07/17 18:08:44  mrimer
// Fixed button hotkey bug.
// Added ALT-ENTER to change screen size.
//
// Revision 1.33  2002/07/05 10:34:12  erikh2000
// Many changes to work with new event-handling scheme.
//
// Revision 1.32  2002/06/25 05:47:28  mrimer
// Added calls to StopRepeat() before activating dialog boxes.
//
// Revision 1.31  2002/06/23 10:55:43  erikh2000
// Revised calls to CFontManager::GetTextRectHeight() to include new param.
//
// Revision 1.30  2002/06/21 22:29:46  erikh2000
// Made the unoverrided Paint() method more useful by having it paint its child widgets.
//
// Revision 1.29  2002/06/21 05:21:02  mrimer
// Replaced MoveFocus() call with InitFocus().
//
// Revision 1.28  2002/06/20 00:51:13  erikh2000
// Moved pre-draw initialization tasks out of Activate() and into new OnBeforeActivate() method.
// Removed paint code from Activate() since first screen paint is now handled by CScreenManager.
//
// Revision 1.27  2002/06/15 23:47:58  erikh2000
// Widget painting methods optionally update their rects now.
//
// Revision 1.26  2002/06/14 01:02:51  erikh2000
// Changed windowed/fullscreen-related methods to use screen surface instead of dest surface.
// Changed a WCHAR buffer parameter to safer wstring.
//
// Revision 1.25  2002/06/13 22:12:26  mrimer
// Minor fix.
//
// Revision 1.24  2002/06/13 21:48:53  mrimer
// Refactored more event code from CScreen to CEventHandlerWidget.
//
// Revision 1.23  2002/06/13 21:08:52  erikh2000
// Moved the input text dialog setup code in a separate section from the message dialog setup code for readability.
//
// Revision 1.22  2002/06/12 22:33:13  mrimer
// Fixed widget bug.
//
// Revision 1.21  2002/06/11 22:53:38  mrimer
// Added ShowTextInputMessage().
// Removed focus and key repeat handling to CEventHandlerWidget.
// Changed "screens" to "destination surfaces".
// Other minor fixes.
//
// Revision 1.20  2002/06/07 17:52:23  mrimer
// Fixed mouse click handling.
//
// Revision 1.19  2002/06/06 00:04:37  mrimer
// Fixed minor bugs.
//
// Revision 1.18  2002/06/05 19:50:51  mrimer
// Inserted focus, hotkey, and key repeat handling (from SettingsScreen.cpp).
//
// Revision 1.17  2002/06/05 03:19:22  mrimer
// Inserted focus vars (from SettingsScreen.cpp).
//
// Revision 1.16  2002/06/03 22:51:55  mrimer
// Added CEventHandlerWidget base class.
//
// Revision 1.15  2002/06/01 04:57:47  mrimer
// Finished implementing hotkeys.
//
// Revision 1.14  2002/05/21 21:42:16  mrimer
// Implemented TextBoxWidget.
//
// Revision 1.13  2002/05/12 03:20:44  erikh2000
// Added OnKeydown() method.
//
// Revision 1.12  2002/05/10 22:41:15  erikh2000
// Moved mouse cursor show/hide code from CGameScreen.
// Moved set windowed/fullscreen code from CWidget.
//
// Revision 1.11  2002/04/29 00:19:01  erikh2000
// Added list box event-handling.
//
// Revision 1.10  2002/04/25 18:18:01  mrimer
// Added quit confirmation message and window resizing event.
//
// Revision 1.9  2002/04/25 18:16:34  mrimer
// Wrote SetFullscreen, SetWindowed, and ToggleScreenSize.
//
// Revision 1.8  2002/04/24 08:14:47  erikh2000
// Added mouse handling for sliders and option buttons.
//
// Revision 1.7  2002/04/19 22:02:42  erikh2000
// Screen now handles painting buttons in response to mouse events with overridable methods.
//
// Revision 1.6  2002/04/16 10:47:19  erikh2000
// Two new methods to show message dialogs added--one for "Okay" messages, the other for "Yes/No" messages.
//
// Revision 1.5  2002/04/14 00:34:13  erikh2000
// Changed calls to GetSurfaceColor().
// Added code to load text from database when displaying message dialog.
//
// Revision 1.4  2002/04/13 19:35:27  erikh2000
// Added methods to show message boxes on screen.
//
// Revision 1.3  2002/04/09 22:15:16  erikh2000
// Added SetWindowed() and SetFullscreen() stubs.
//
// Revision 1.2  2002/04/09 10:05:39  erikh2000
// Fixed revision log macro.
//
