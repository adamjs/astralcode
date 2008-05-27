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
*/

#ifndef __BrowserWindowImpl_H__
#define __BrowserWindowImpl_H__

#include "BrowserWindow.h"
#include <vector>
#include "windows.h"

#include "nsIInterfaceRequestor.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWebProgressListener.h"
#include "nsIURIContentListener.h"
#include "nsIDOMEventListener.h"
#include "nsWeakReference.h"
#include "nsIWebBrowser.h"
#include "nsIToolkit.h"

class nsIViewManager;

namespace Astral {
namespace Impl {

struct IntRect { int left, right, top, bottom; };

class RenderBuffer
{
public:
	unsigned char* buffer;
	int width, height, rowSpan;

	RenderBuffer(int width, int height);

	~RenderBuffer();

	void reserve(int width, int height);
	void copyFrom(unsigned char* srcBuffer, int srcRowSpan);
	void copyArea(IntRect srcRect, unsigned char* srcBuffer, int srcRowSpan);
	void blitBGR(unsigned char* destBuffer, int destRowSpan, int destDepth);
};

struct CaretState
{
	bool isVisible;
	int x, y, height;

	CaretState();
	CaretState(bool isVisible, int x, int y, int height);

	bool operator==(const CaretState& rhs) const;
};

class CaretRenderer
{
	std::vector<unsigned char> overwrittenPixels;
	IntRect bounds;
public:
	CaretRenderer();

	void renderCaret(const CaretState& state, unsigned char R, unsigned char G, unsigned char B, RenderBuffer& context);

	void clearCaret(RenderBuffer& context);
};

class ContentListener;

class BrowserWindowImpl : public BrowserWindow, public nsIInterfaceRequestor, public nsIWebBrowserChrome, public nsIWebProgressListener,
	public nsIURIContentListener, public nsSupportsWeakReference, public nsIToolkitObserver
{
protected:
	nsCOMPtr<nsIWebBrowser> webBrowser;
	nsCOMPtr<nsIToolkit> activeToolkit;
	std::vector<BrowserWindowListener*> listeners;
	typedef std::vector<BrowserWindowListener*>::iterator ListenerIter;
	std::string currentURL;
	bool isLoaded;
	std::pair<int,int> dimensions;
	IntRect dirtyBounds;
	bool isClean;
	bool isTotallyDirty;
	RenderBuffer renderBuffer;
	CaretState curCaret;
	CaretRenderer caretRenderer;
	bool isCaretDirty;
	
	nsIViewManager* getViewManager();
	CaretState determineCaretState();
	void sendMouseEvent(short eventCode, short x, short y, int numClicks);
	void sendKeyboardEvent(int unicodeChar, int keyCode);
public:
	BrowserWindowImpl(int width, int height);
	~BrowserWindowImpl();

	NS_DECL_ISUPPORTS
	NS_DECL_NSIINTERFACEREQUESTOR
	NS_DECL_NSIWEBBROWSERCHROME
	NS_DECL_NSIWEBPROGRESSLISTENER
	NS_DECL_NSIURICONTENTLISTENER
	NS_DECL_NSITOOLKITOBSERVER
	
	void destroy();

	void navigateTo(const std::string& url);
	void navigateStop();
	void navigateReload();
	void navigateForward();
	void navigateBack();
	
	bool canNavigateForward() const;
	bool canNavigateBack() const;

	std::string evaluateJS(const std::string& script);

	void focus();
	void defocus();

	void injectMouseMove(short x, short y);
	void injectMouseDown(short x, short y);
	void injectMouseUp(short x, short y);
	void injectMouseDblClick(short x, short y);
	void injectKeyPress(int keyCode);
	void injectKeyUnicode(int unicodeChar);
	void injectScroll(short numLines);

	void addListener(BrowserWindowListener* listener);
	void removeListener(BrowserWindowListener* listener);

	bool isLoading() const;
	bool isDirty();

	bool render(unsigned char* destination, int destRowSpan, int destDepth, bool force = false);

	std::string getCurrentURL() const;

	std::pair<int,int> getDimensions() const;
	
	void resize(short width, short height);
};

}
}

#endif