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

#include "Astral.h"
#include "BrowserWindowImpl.h"
#include "WindowCreatorImpl.h"

#include "nsCOMPtr.h"
#include "nsXULAppAPI.h"
#include "nsXPCOMGlue.h"
#include "nsILocalFile.h"
#include "nsString.h"
#include "nsIPref.h"
#include "nsICacheService.h"
#include "nsNetCID.h"
#include "nsIWindowWatcher.h"

using namespace Astral;

namespace Impl {
	class WindowCreator;
}

AstralManager* AstralManager::instance = 0;

AstralManager::AstralManager(const std::string& runtimeDir, void* windowHandle) : windowHandle(windowHandle)
{
	instance = this;

	nsresult result;

	nsCOMPtr<nsILocalFile> xuldir;
	result = NS_NewNativeLocalFile(nsCString(runtimeDir.c_str()), PR_FALSE, getter_AddRefs(xuldir));
	if(NS_FAILED(result))
		throw std::exception("Could not create AstralManager: invalid runtime directory.");

	nsCOMPtr<nsILocalFile> appdir;
	result = NS_NewNativeLocalFile(nsCString(runtimeDir.c_str()), PR_FALSE, getter_AddRefs(appdir));
	if (NS_FAILED(result))
		throw std::exception("Could not create AstralManager: invalid runtime directory.");

	result = XRE_InitEmbedding(xuldir, appdir, 0, 0, 0);
	if(NS_FAILED(result))
		throw std::exception("Could not create AstralManager: missing components or invalid runtime directory.");

	nsCOMPtr<nsIWindowCreator> creator(new Impl::WindowCreator());
	if(!creator)
		return;

	nsCOMPtr<nsIWindowWatcher> wwatcher(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
	if(!wwatcher)
		return;

	wwatcher->SetWindowCreator(creator);

	setBooleanPref("security.warn_entering_secure", false);
	setBooleanPref("security.warn_entering_weak", false);
	setBooleanPref("security.warn_leaving_secure", false);
	setBooleanPref("security.warn_submit_insecure", false);
	setBooleanPref("network.protocol-handler.warn-external-default", false);
}

AstralManager::~AstralManager()
{
	for(WindowIter i = windows.begin(); i != windows.end(); i++)
		(*i)->destroy();

	nsCOMPtr<nsIWindowWatcher> wwatcher(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
	if(wwatcher)
	{
		wwatcher->SetWindowCreator(0);
		wwatcher = 0;
	}

	XRE_TermEmbedding();

	instance = 0;
}

AstralManager& AstralManager::Get()
{
	if(!instance)
		throw std::exception("In AstralManager::Get(), AstralManager is uninitialized!");

	return *instance;
}

AstralManager* AstralManager::GetPointer()
{
	return instance;
}

BrowserWindow* AstralManager::createBrowserWindow(int width, int height)
{
	return new Impl::BrowserWindowImpl(width, height);
}

bool AstralManager::setBooleanPref(const std::string& prefName, bool value)
{
	nsresult result;
	nsCOMPtr<nsIPref> pref = do_CreateInstance(NS_PREF_CONTRACTID, &result);
	if(NS_FAILED(result))
		return false;

	result = pref->SetBoolPref(prefName.c_str(), (PRBool)value);

	return NS_SUCCEEDED(result);
}

bool AstralManager::setIntegerPref(const std::string& prefName, int value)
{
	nsresult result;
	nsCOMPtr<nsIPref> pref = do_CreateInstance(NS_PREF_CONTRACTID, &result);
	if(NS_FAILED(result))
		return false;

	result = pref->SetIntPref(prefName.c_str(), (PRInt32)value);

	return NS_SUCCEEDED(result);
}

bool AstralManager::setStringPref(const std::string& prefName, const std::string& value)
{
	nsresult result;
	nsCOMPtr<nsIPref> pref = do_CreateInstance(NS_PREF_CONTRACTID, &result);
	if(NS_FAILED(result))
		return false;

	result = pref->SetCharPref(prefName.c_str(), value.c_str());

	return NS_SUCCEEDED(result);
}

void AstralManager::defocusAll()
{
	for(WindowIter i = windows.begin(); i != windows.end(); i++)
		(*i)->defocus();
}

void AstralManager::clearCache()
{
	nsresult result;
	nsCOMPtr<nsICacheService> cacheService = do_GetService(NS_CACHESERVICE_CONTRACTID, &result);
	if(NS_FAILED(result))
		return;

	cacheService->EvictEntries(nsICache::STORE_ANYWHERE);
}

