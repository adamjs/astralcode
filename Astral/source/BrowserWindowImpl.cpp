/*
	This file is part of Astral, a wrapper for Mozilla Gecko, designed explicitly for NaviLibrary.

	The contents of this file are subject to the Mozilla Public License
	Version 1.1 (the "MPL License"); you may not use this file except in
	compliance with the License. You may obtain a copy of the License at
	http://www.mozilla.org/MPL/

	Software distributed under the License is distributed on an "AS IS"
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
	License for the specific language governing rights and limitations
	under the License.

	The Original Code is Copyright (C) 2008 Adam J. Simmons.

	The Initial Developer of the Original Code is Adam J. Simmons.
	Portions created by the Initial Developer are Copyright (C) 2008
	by the Initial Developer. All Rights Reserved.
	
	Portions of the code are derived from LLMozLib (http://ubrowser.com/) 
	and are Copyright (C) 2006 Callum Prentice and Linden Lab Inc.
*/

#include "BrowserWindowImpl.h"
#include "Astral.h"
#include <algorithm>

#include "nsString.h"
#include "nsStringAPI.h"
#include "nsIHttpChannel.h"
#include "nsIURI.h"
#include "nsIDOMEvent.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMElement.h"
#include "nsIDOMWindow.h"
#include "nsIBaseWindow.h"
#include "nsIComponentManager.h"
#include "nsEmbedCID.h"
#include "nsIWebNavigation.h"
#include "nsIScriptGlobalObjectOwner.h"
#include "nsIScriptGlobalObject.h"
#include "nsIScriptContext.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIWebBrowserFocus.h"
#include "nsIDocShell.h"
#include "nsIPresShell.h"
#include "nsIViewManager.h"
#include "nsIScrollableView.h"
#include "nsIDrawingSurface.h"
#include "nsIRenderingContext.h"
#include "nsIToolkit.h"
#include "nsGUIEvent.h"
#include "nsIDOMDocument.h"
#include "nsIDocument.h"
#include "nsIContent.h"
#include "nsISelectionController.h"
#include "nsIFrame.h"
#include "nsICaret.h"

using namespace Astral;
using namespace Astral::Impl;


RenderBuffer::RenderBuffer(int width, int height) : width(0), height(0), buffer(0), rowSpan(0)
{
	reserve(width, height);
}

RenderBuffer::~RenderBuffer()
{
	if(buffer)
		delete[] buffer;
}

void RenderBuffer::reserve(int width, int height)
{
	if(this->width != width || this->height != height)
	{
		this->width = width;
		this->height = height;

		rowSpan = width * 4;

		if(buffer)
			delete[] buffer;

		buffer = new unsigned char[width * height * 4];
		memset(buffer, 255, width * height * 4);
	}
}

void RenderBuffer::copyFrom(unsigned char* srcBuffer, int srcRowSpan)
{
	for(int row = 0; row < height; row++)
		memcpy(buffer + row * rowSpan, srcBuffer + row * srcRowSpan, rowSpan);
}

void RenderBuffer::copyArea(IntRect srcRect, unsigned char* srcBuffer, int srcRowSpan)
{
	if(srcRect.right <= width && srcRect.bottom <= height)
	{
		int srcWidth = srcRect.right - srcRect.left;
		int srcHeight = srcRect.bottom - srcRect.top;

		for(int row = 0; row < srcHeight; row++)
			memcpy(buffer + (row + srcRect.top) * rowSpan + (srcRect.left * 4), srcBuffer + row * srcRowSpan, srcWidth * 4);
	}
}

void RenderBuffer::blitBGR(unsigned char* destBuffer, int destRowSpan, int destDepth)
{
	if(destDepth == 3)
	{
		for(int row = 0; row < height; row++)
			for(int col = 0; col < width; col++)
				memcpy(destBuffer + row * destRowSpan + col * 3, buffer + row * rowSpan + col * 4, 3);
	}
	else if(destDepth == 4)
	{
		for(int row = 0; row < height; row++)
			memcpy(destBuffer + row * destRowSpan, buffer + row * rowSpan, rowSpan);
	}
}

CaretState::CaretState() : isVisible(false), x(0), y(0), height(0)
{
}

CaretState::CaretState(bool isVisible, int x, int y, int height) : isVisible(isVisible), x(x), y(y), height(height)
{
}

bool CaretState::operator==(const CaretState& rhs) const
{
	return isVisible == rhs.isVisible && x == rhs.x && y == rhs.y && height == rhs.height;
}

CaretRenderer::CaretRenderer()
{
}

void CaretRenderer::renderCaret(const CaretState& state, unsigned char R, unsigned char G, unsigned char B, RenderBuffer& context)
{
	if(!state.isVisible)
		return;
	else if(state.x < 0 || state.y < 0 || state.x + 1 > context.width || state.y + state.height > context.height)
		return;

	overwrittenPixels.clear();
	bounds.left = state.x;
	bounds.right = state.x + 1;
	bounds.top = state.y;
	bounds.bottom = state.y + state.height;

	unsigned char* p = 0;
	for(int row = bounds.top; row < bounds.bottom; row++)
	{
		p = context.buffer + row * context.rowSpan + bounds.left * 4;

		overwrittenPixels.push_back(*p);
		*p++ = B;
		overwrittenPixels.push_back(*p);
		*p++ = G;
		overwrittenPixels.push_back(*p);
		*p++ = R;
		overwrittenPixels.push_back(*p);
		*p++ = 255;
	}
}

void CaretRenderer::clearCaret(RenderBuffer& context)
{
	if(!overwrittenPixels.size())
		return;

	context.copyArea(bounds, &overwrittenPixels[0], 4);

	overwrittenPixels.clear();
}

BrowserWindowImpl::BrowserWindowImpl(int width, int height) : isLoaded(true), isClean(true), isTotallyDirty(false), isCaretDirty(false), renderBuffer(width, height)
{
	this->AddRef();

	dimensions.first = width;
	dimensions.second = height;

	webBrowser = do_CreateInstance(NS_WEBBROWSER_CONTRACTID);
	webBrowser->SetContainerWindow(this);

	nsCOMPtr<nsIBaseWindow> baseWindow = do_QueryInterface(webBrowser);
	baseWindow->InitWindow(AstralManager::Get().windowHandle, 0, 0, 0, width, height);
	baseWindow->Create();
	baseWindow->SetPosition(-5000, 0);
	baseWindow->SetSize(width, height, false);
	baseWindow->SetVisibility(PR_FALSE);

	nsCOMPtr<nsIWebProgressListener> listener(this);
    nsCOMPtr<nsIWeakReference> weakListener(do_GetWeakReference(listener));
    webBrowser->AddWebBrowserListener(weakListener, NS_GET_IID(nsIWebProgressListener));

	webBrowser->SetParentURIContentListener(this);
	
	AstralManager::Get().windows.push_back(this);
}

BrowserWindowImpl::~BrowserWindowImpl()
{
}

NS_IMPL_ADDREF(BrowserWindowImpl)
NS_IMPL_RELEASE(BrowserWindowImpl)

NS_INTERFACE_MAP_BEGIN(BrowserWindowImpl)
	NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIWebBrowserChrome)
	NS_INTERFACE_MAP_ENTRY(nsIInterfaceRequestor)
	NS_INTERFACE_MAP_ENTRY(nsIWebBrowserChrome)
	NS_INTERFACE_MAP_ENTRY(nsIWebProgressListener)
	NS_INTERFACE_MAP_ENTRY(nsIURIContentListener)
	NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
	NS_INTERFACE_MAP_ENTRY(nsIToolkitObserver)
NS_INTERFACE_MAP_END

// nsIInterfaceRequestor Definitions

NS_IMETHODIMP BrowserWindowImpl::GetInterface(const nsIID & uuid, void * *result)
{
	if(uuid.Equals(NS_GET_IID(nsIDOMWindow)))
	{
		if(webBrowser)
			return webBrowser->GetContentDOMWindow((nsIDOMWindow**)result);

		return NS_ERROR_NOT_INITIALIZED;
	}

	return QueryInterface(uuid, result);
}

// Begin nsIWebBrowserChrome Definitions

NS_IMETHODIMP BrowserWindowImpl::SetStatus(PRUint32 statusType, const PRUnichar *status)
{
	std::string statusText(NS_ConvertUTF16toUTF8(status).get());

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
		(*i)->onStatusTextChange(this, statusText);

	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::GetWebBrowser(nsIWebBrowser * *aWebBrowser)
{
	NS_ENSURE_ARG_POINTER(aWebBrowser);
	*aWebBrowser = webBrowser;
	NS_IF_ADDREF(*aWebBrowser);

	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::SetWebBrowser(nsIWebBrowser * aWebBrowser)
{
	NS_ENSURE_ARG_POINTER(aWebBrowser);
	webBrowser = aWebBrowser;

	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::GetChromeFlags(PRUint32 *aChromeFlags)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP BrowserWindowImpl::SetChromeFlags(PRUint32 aChromeFlags)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP BrowserWindowImpl::DestroyBrowserWindow(void)
{
	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::SizeBrowserTo(PRInt32 aCX, PRInt32 aCY)
{
	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::ShowAsModal(void)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP BrowserWindowImpl::IsWindowModal(PRBool *_retval)
{
	*_retval = PR_FALSE;

	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::ExitModalEventLoop(nsresult aStatus)
{
	return NS_OK;
}

// Begin nsIWebProgressListener Definitions

NS_IMETHODIMP BrowserWindowImpl::OnStateChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aStateFlags, nsresult aStatus)
{
 	if((aStateFlags & STATE_START) && (aStateFlags & STATE_IS_DOCUMENT) && (aStatus == NS_OK))
	{
		if(activeToolkit)
		{
			activeToolkit->RemoveObserver(this);
			activeToolkit = 0;
		}

		nsIViewManager* viewManager = getViewManager();
		if(viewManager)
		{
			nsIView* view;
			viewManager->GetRootView(view);
			if(view)
			{
				activeToolkit = view->GetWidget()->GetToolkit();
				activeToolkit->AddObserver(this);
			}
		}

		isLoaded = false;
	}
	else if((aStateFlags & STATE_STOP) && (aStateFlags & STATE_IS_WINDOW) && (aStatus == NS_OK))
	{
		PRUint32 responseStatus = 0;
		if(aRequest)
		{
			nsresult result;
			nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(aRequest, &result);
			if(NS_SUCCEEDED(result))
				httpChannel->GetResponseStatus(&responseStatus);
		}

		isLoaded = true;

		for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
			(*i)->onNavigateComplete(this, getCurrentURL(), (int)responseStatus);
	}

	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::OnProgressChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress, PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
{
	short percentComplete = (short)((aCurTotalProgress * 100.0f) / (float)aMaxTotalProgress);

	if(percentComplete < 0)
		percentComplete = 0;
	else if(percentComplete > 100)
		percentComplete = 100;

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
		(*i)->onUpdateProgress(this, percentComplete);

	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::OnLocationChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsIURI *aLocation)
{
	nsCAutoString newURI;
	aLocation->GetSpec(newURI);
	currentURL = newURI.get();

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
		(*i)->onLocationChange(this, currentURL);

	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::OnStatusChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsresult aStatus, const PRUnichar *aMessage)
{
	std::string statusText(NS_ConvertUTF16toUTF8(aMessage).get());

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
		(*i)->onStatusTextChange(this, statusText);

	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::OnSecurityChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aState)
{
	return NS_OK;
}

// Begin nsIURIContentListener definitions

NS_IMETHODIMP BrowserWindowImpl::OnStartURIOpen(nsIURI *aURI, PRBool *_retval)
{
	nsCAutoString urlString;
	aURI->GetSpec(urlString);
	std::string url(urlString.get());

	bool abortURI = false;

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
	{
		bool continueTest = true;
		(*i)->onNavigateBegin(this, url, continueTest);

		if(!continueTest)
			abortURI = true;
	}

	*_retval = abortURI;

	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::DoContent(const char *aContentType, PRBool aIsContentPreferred, nsIRequest *aRequest, nsIStreamListener **aContentHandler, PRBool *_retval)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP BrowserWindowImpl::IsPreferred(const char *aContentType, char **aDesiredContentType, PRBool *_retval)
{
	*_retval = PR_TRUE;
	return NS_OK;
}

NS_IMETHODIMP BrowserWindowImpl::CanHandleContent(const char *aContentType, PRBool aIsContentPreferred, char **aDesiredContentType, PRBool *_retval)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP BrowserWindowImpl::GetLoadCookie(nsISupports * *aLoadCookie)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP BrowserWindowImpl::SetLoadCookie(nsISupports * aLoadCookie)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP BrowserWindowImpl::GetParentContentListener(nsIURIContentListener * *aParentContentListener)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP BrowserWindowImpl::SetParentContentListener(nsIURIContentListener * aParentContentListener)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

// Begin BrowserWindow Definitions

void BrowserWindowImpl::destroy()
{
	std::vector<BrowserWindow*>& windows = AstralManager::Get().windows;

	AstralManager::WindowIter iter = std::find(windows.begin(), windows.end(), this);
	if(iter != windows.end())
		windows.erase(iter);

	nsCOMPtr<nsIWebBrowser> browser = nsnull;
	this->GetWebBrowser(getter_AddRefs(browser));
	nsCOMPtr<nsIBaseWindow> browserAsWin = do_QueryInterface(browser);

	if(activeToolkit)
	{
		activeToolkit->RemoveObserver(this);
		activeToolkit = 0;
	}

	browser->SetParentURIContentListener(0);

	navigateStop();
	
	if(browserAsWin)
		browserAsWin->Destroy();

	this->SetWebBrowser(nsnull);
	this->Release();
}

void BrowserWindowImpl::navigateTo(const std::string& url)
{
	nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(webBrowser));
	webNav->LoadURI(NS_ConvertASCIItoUTF16(url.c_str()).get(), nsIWebNavigation::LOAD_FLAGS_NONE, 0, 0, 0);
}

void BrowserWindowImpl::navigateStop()
{
	nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(webBrowser));
	webNav->Stop(nsIWebNavigation::STOP_ALL);
}

void BrowserWindowImpl::navigateReload()
{
	nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(webBrowser));
	webNav->Reload(nsIWebNavigation::LOAD_FLAGS_NONE);
}

void BrowserWindowImpl::navigateForward()
{
	nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(webBrowser));
	webNav->GoForward();
}

void BrowserWindowImpl::navigateBack()
{
	nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(webBrowser));
	webNav->GoBack();
}

bool BrowserWindowImpl::canNavigateForward() const
{
	nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(webBrowser));
	PRBool result = PR_FALSE;
	webNav->GetCanGoForward(&result);

	return result ? true : false;
}

bool BrowserWindowImpl::canNavigateBack() const
{
	nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(webBrowser));
	PRBool result = PR_FALSE;
	webNav->GetCanGoBack(&result);

	return result ? true : false;
}

std::string BrowserWindowImpl::evaluateJS(const std::string& script)
{
	nsresult result;

	nsCOMPtr<nsIScriptGlobalObjectOwner> scriptOwner(do_GetInterface(webBrowser, &result));
	if(NS_FAILED(result))
		return "";

	nsIScriptContext* context = scriptOwner->GetScriptGlobalObject()->GetContext();

	PRBool undefined;
	nsString returnVal;
	result = context->EvaluateString(NS_ConvertUTF8toUTF16(script.c_str()), 0, 0, "", 1, 0, &returnVal, &undefined);
	if(NS_FAILED(result))
		return "";

	return std::string(NS_ConvertUTF16toUTF8(returnVal).get());
}

void BrowserWindowImpl::focus()
{
	nsCOMPtr<nsIWebBrowserFocus> focus(do_GetInterface(webBrowser));
	focus->Activate();
}

void BrowserWindowImpl::defocus()
{
	nsCOMPtr<nsIWebBrowserFocus> focus(do_GetInterface(webBrowser));
	focus->Deactivate();
}

void BrowserWindowImpl::injectMouseMove(short x, short y)
{
	sendMouseEvent(NS_MOUSE_MOVE, x, y, 1);
}

void BrowserWindowImpl::injectMouseDown(short x, short y)
{
	sendMouseEvent(NS_MOUSE_LEFT_BUTTON_DOWN, x, y, 1);
}

void BrowserWindowImpl::injectMouseUp(short x, short y)
{
	sendMouseEvent(NS_MOUSE_LEFT_BUTTON_UP, x, y, 1);
}

void BrowserWindowImpl::injectMouseDblClick(short x, short y)
{
	sendMouseEvent(NS_MOUSE_LEFT_BUTTON_DOWN, x, y, 2);
}

void BrowserWindowImpl::injectKeyPress(int keyCode)
{
	sendKeyboardEvent(0, keyCode);
}

void BrowserWindowImpl::injectKeyUnicode(int unicodeChar)
{
	sendKeyboardEvent(unicodeChar, 0);
}

void BrowserWindowImpl::injectScroll(short numLines)
{
	nsCOMPtr<nsIDOMWindow> domWindow;
	nsresult result = webBrowser->GetContentDOMWindow(getter_AddRefs(domWindow));

	if(NS_SUCCEEDED(result))
		domWindow->ScrollByLines(numLines);
}

void BrowserWindowImpl::addListener(BrowserWindowListener* listener)
{
	ListenerIter iter = std::find(listeners.begin(), listeners.end(), listener);

	if(iter == listeners.end())
		listeners.push_back(listener);
}

void BrowserWindowImpl::removeListener(BrowserWindowListener* listener)
{
	ListenerIter iter = std::find(listeners.begin(), listeners.end(), listener);

	if(iter != listeners.end())
		listeners.erase(iter);
}

bool BrowserWindowImpl::isLoading() const
{
	return !isLoaded;
}

bool BrowserWindowImpl::isDirty()
{
	CaretState newCaret = determineCaretState();
	isCaretDirty = !(newCaret == curCaret);
	curCaret = newCaret;

	return !isClean || isCaretDirty;
}

bool BrowserWindowImpl::render(unsigned char* destination, int destRowSpan, int destDepth, bool force)
{
	if(force)
	{
		isClean = false;
		isTotallyDirty = true;
		dirtyBounds.left = 0;
		dirtyBounds.top = 0;
		dirtyBounds.right = dimensions.first;
		dirtyBounds.bottom = dimensions.second;
	}
	else if(isClean)
	{
		if(isCaretDirty)
		{
			caretRenderer.clearCaret(renderBuffer);
			if(curCaret.isVisible) caretRenderer.renderCaret(curCaret, 0, 0, 0, renderBuffer);
			renderBuffer.blitBGR(destination, destRowSpan, destDepth);

			return true;
		}
		else
			return false;
	}

	nsresult res;

	nsIViewManager* viewManager = getViewManager();
	if(!viewManager)
		return false;

	nsIScrollableView* scrollableView = NULL;
	viewManager->GetRootScrollableView(&scrollableView);
	nsIView* view = 0;
	
	if(scrollableView)
		scrollableView->GetScrolledView(view);
	else
		viewManager->GetRootView(view);

	nsRect viewRect = view->GetBounds() - view->GetPosition() - view->GetPosition();

	nsRect rect;
	rect.x = viewRect.x + (dirtyBounds.left * 15);
	rect.y = viewRect.y + (dirtyBounds.top * 15);
	rect.width = (dirtyBounds.right - dirtyBounds.left) * 15;
	rect.height = (dirtyBounds.bottom - dirtyBounds.top) * 15;

	int width = dirtyBounds.right - dirtyBounds.left;
	int height = dirtyBounds.bottom - dirtyBounds.top;

	nsCOMPtr<nsIRenderingContext> context;
	res = viewManager->RenderOffscreen(view, rect, PR_FALSE, PR_FALSE, NS_RGB(255, 255, 255), getter_AddRefs(context));
	if(NS_FAILED(res))
		return false;

	nsIDrawingSurface* surface = 0;
	context->GetDrawingSurface(&surface);
	if(!surface)
		return false;

	PRUint8* source;
	PRInt32 rowLen;
	PRInt32 rowSpan;

	res = surface->Lock(0, 0, width, height, (void**)&source, &rowSpan, &rowLen, NS_LOCK_SURFACE_READ_ONLY);
	if(NS_FAILED(res))
		return false;

	if(isTotallyDirty)
	{
		renderBuffer.copyFrom(source, rowSpan);
	}
	else
	{
		if(isCaretDirty)
			caretRenderer.clearCaret(renderBuffer);

		renderBuffer.copyArea(dirtyBounds, source, rowSpan);
	}

	surface->Unlock();
	context->DestroyDrawingSurface(surface);

	if(isCaretDirty && curCaret.isVisible)
		caretRenderer.renderCaret(curCaret, 0, 0, 0, renderBuffer);

	renderBuffer.blitBGR(destination, destRowSpan, destDepth);

	isClean = true;
	isTotallyDirty = false;

	return true;
}

std::string BrowserWindowImpl::getCurrentURL() const
{
	return currentURL;
}

std::pair<int,int> BrowserWindowImpl::getDimensions() const
{
	return dimensions;
}

void BrowserWindowImpl::resize(short width, short height)
{
	dimensions.first = width;
	dimensions.second = height;
	renderBuffer.reserve(width, height);

	nsCOMPtr<nsIBaseWindow> baseWindow = do_QueryInterface(webBrowser);
	baseWindow->SetSize(width, height, PR_FALSE);

	isClean = false;
	isTotallyDirty = true;
	dirtyBounds.left = 0;
	dirtyBounds.top = 0;
	dirtyBounds.right = dimensions.first;
	dirtyBounds.bottom = dimensions.second;
}

NS_METHOD BrowserWindowImpl::NotifyInvalidated(nsIWidget *aWidget, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height)
{
	if(isTotallyDirty)
		return NS_OK;

	nsIWidget* mainWidget = 0;
	nsCOMPtr<nsIBaseWindow> baseWindow = do_QueryInterface(webBrowser);
	baseWindow->GetMainWidget(&mainWidget);

	HWND nativeWidget = (HWND)aWidget->GetNativeData(NS_NATIVE_WIDGET);
	HWND nativeWidgetChild = 0;
	while(GetParent(nativeWidget))
	{
		nativeWidgetChild = nativeWidget;
		nativeWidget = GetParent(nativeWidget);
	}

	if(((HWND)mainWidget->GetNativeData(NS_NATIVE_WIDGET)) != nativeWidgetChild)
		return NS_OK;

	if(isClean)
	{

		dirtyBounds.left = x;
		dirtyBounds.right = x + width;
		dirtyBounds.top = y;
		dirtyBounds.bottom = y + height;
		isClean = false;
	}
	else
	{
		dirtyBounds.left = min(dirtyBounds.left, x);
		dirtyBounds.right = max(dirtyBounds.right, x + width);
		dirtyBounds.top = min(dirtyBounds.top, y);
		dirtyBounds.bottom = max(dirtyBounds.bottom, y + height);
	}

	if(dirtyBounds.left < 0 || dirtyBounds.top < 0 || dirtyBounds.right > dimensions.first || dirtyBounds.bottom > dimensions.second)
	{
		isTotallyDirty = true;
		dirtyBounds.left = 0;
		dirtyBounds.top = 0;
		dirtyBounds.right = dimensions.first;
		dirtyBounds.bottom = dimensions.second;
	}

	return NS_OK;
}

nsIViewManager* BrowserWindowImpl::getViewManager()
{
	nsresult res;

	nsCOMPtr<nsIDocShell> docShell = do_GetInterface(webBrowser);
	nsCOMPtr<nsIPresShell> presShell;
	res = docShell->GetPresShell(getter_AddRefs(presShell));
	if(!NS_FAILED(res))
		return presShell->GetViewManager();

	return 0;
}

CaretState BrowserWindowImpl::determineCaretState()
{
	nsCOMPtr<nsIWebBrowserFocus> browserFocus = do_QueryInterface(webBrowser);

	nsCOMPtr<nsIDOMElement> focusedElement;
	browserFocus->GetFocusedElement(getter_AddRefs(focusedElement));
	if(!focusedElement) return CaretState();

	nsCOMPtr<nsIContent> focusedContent = do_QueryInterface(focusedElement);

	nsCOMPtr<nsIDOMWindow> domWindow;
	webBrowser->GetContentDOMWindow(getter_AddRefs(domWindow));
	if(!domWindow) return CaretState();

	nsCOMPtr<nsIDOMDocument> domDocument;
	domWindow->GetDocument(getter_AddRefs(domDocument));
	if(!domDocument) return CaretState();

	nsCOMPtr<nsIDocument> document = do_QueryInterface(domDocument);
	if(!document) return CaretState();

	nsIPresShell* presShell = document->GetShellAt(0);
	if(!presShell) return CaretState();

	nsCOMPtr<nsICaret> caret;
	presShell->GetCaret(getter_AddRefs(caret));

	nsIFrame* frame = 0;
	presShell->GetPrimaryFrameFor(focusedContent, &frame);
	if(!frame) return CaretState();

	nsCOMPtr<nsISelectionController> selCtrl;
	frame->GetSelectionController(presShell->GetPresContext(), getter_AddRefs(selCtrl));

	nsCOMPtr<nsISelection> selection;
	selCtrl->GetSelection(nsISelectionController::SELECTION_NORMAL, getter_AddRefs(selection));

	PRBool collapsed;
	nsRect coords;
	nsIView* caretView;
	caret->GetCaretCoordinates(nsICaret::eTopLevelWindowCoordinates, selection, &coords, &collapsed, &caretView);

	float t2p = presShell->GetPresContext()->TwipsToPixels();

	PRInt32 caretX = coords.x / 15;
	PRInt32 caretY = coords.y / 15;
	PRInt32 caretHeight = coords.height / 15;

	return CaretState(true, caretX, caretY, caretHeight);
}

void BrowserWindowImpl::sendMouseEvent(short eventCode, short x, short y, int numClicks)
{
	nsresult result;

	nsIViewManager* viewManager = getViewManager();
	if(!viewManager)
		return;

	nsIView* rootView;
	result = viewManager->GetRootView(rootView);
	if(NS_FAILED(result))
		return;

	nsCOMPtr<nsIWidget> widget = rootView->GetWidget();
	if(!widget)
		return;

	nsMouseEvent event(true, eventCode, widget, nsMouseEvent::eReal);
	event.clickCount = numClicks;
	event.isShift = 0;
	event.isControl = 0;
	event.isAlt = 0;
	event.isMeta = 0;
	event.widget = widget;
	event.nativeMsg = 0;
	event.point.x = x;
	event.point.y = y;
	event.refPoint.x = x;
	event.refPoint.y = y;
	event.flags = 0;

	nsEventStatus status;
	viewManager->DispatchEvent(&event, &status);
}

void BrowserWindowImpl::sendKeyboardEvent(int unicodeChar, int keyCode)
{
	nsresult result;

	nsIViewManager* viewManager = getViewManager();
	if(!viewManager)
		return;

	nsIView* rootView;
	result = viewManager->GetRootView(rootView);
	if(NS_FAILED(result))
		return;

	nsCOMPtr<nsIWidget> widget = rootView->GetWidget();
	if(!widget)
		return;

	nsKeyEvent event(true, NS_KEY_PRESS, widget);
	event.keyCode = keyCode;
	event.charCode = unicodeChar;
	event.isChar = unicodeChar ? true : false;
	event.isShift = 0;
	event.isControl = 0;
	event.isAlt = 0;
	event.isMeta = 0;
	event.widget = widget;
	event.nativeMsg = 0;
	event.point.x = 0;
	event.point.y = 0;
	event.refPoint.x = 0;
	event.refPoint.y = 0;
	event.flags = 0;

	nsEventStatus status;
	viewManager->DispatchEvent(&event, &status);
}