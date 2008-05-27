/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM g:/mozilla/widget/public/nsIFilePicker.idl
 */

#ifndef __gen_nsIFilePicker_h__
#define __gen_nsIFilePicker_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsILocalFile; /* forward declaration */

class nsIFileURL; /* forward declaration */

class nsIDOMWindow; /* forward declaration */

class nsISimpleEnumerator; /* forward declaration */


/* starting interface:    nsIFilePicker */
#define NS_IFILEPICKER_IID_STR "80faf095-c807-4558-a2cc-185ed70754ea"

#define NS_IFILEPICKER_IID \
  {0x80faf095, 0xc807, 0x4558, \
    { 0xa2, 0xcc, 0x18, 0x5e, 0xd7, 0x07, 0x54, 0xea }}

class NS_NO_VTABLE nsIFilePicker : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IFILEPICKER_IID)

  enum { modeOpen = 0 };

  enum { modeSave = 1 };

  enum { modeGetFolder = 2 };

  enum { modeOpenMultiple = 3 };

  enum { returnOK = 0 };

  enum { returnCancel = 1 };

  enum { returnReplace = 2 };

  enum { filterAll = 1 };

  enum { filterHTML = 2 };

  enum { filterText = 4 };

  enum { filterImages = 8 };

  enum { filterXML = 16 };

  enum { filterXUL = 32 };

  enum { filterApps = 64 };

  /**
  * Initialize the file picker widget.  The file picker is not valid until this
  * method is called.
  *
  * @param      parent   nsIDOMWindow parent.  This dialog will be dependent
  *                      on this parent. parent must be non-null.
  * @param      title    The title for the file widget
  * @param      mode     load, save, or get folder
  *
  */
  /* void init (in nsIDOMWindow parent, in AString title, in short mode); */
  NS_IMETHOD Init(nsIDOMWindow *parent, const nsAString & title, PRInt16 mode) = 0;

  /**
  * Append to the  filter list with things from the predefined list
  *
  * @param      filters  mask of filters i.e. (filterAll | filterHTML)
  *
  */
  /* void appendFilters (in long filterMask); */
  NS_IMETHOD AppendFilters(PRInt32 filterMask) = 0;

  /**
  * Add a filter
  *
  * @param      title    name of the filter
  * @param      filter   extensions to filter -- semicolon and space separated
  *
  */
  /* void appendFilter (in AString title, in AString filter); */
  NS_IMETHOD AppendFilter(const nsAString & title, const nsAString & filter) = 0;

  /**
  * The filename that should be suggested to the user as a default.
  *
  * @throws NS_ERROR_FAILURE on attempts to get
  */
  /* attribute AString defaultString; */
  NS_IMETHOD GetDefaultString(nsAString & aDefaultString) = 0;
  NS_IMETHOD SetDefaultString(const nsAString & aDefaultString) = 0;

  /**
  * The extension that should be associated with files of the type we
  * want to work with.  On some platforms, this extension will be
  * automatically appended to filenames the user enters, if needed.  
  */
  /* attribute AString defaultExtension; */
  NS_IMETHOD GetDefaultExtension(nsAString & aDefaultExtension) = 0;
  NS_IMETHOD SetDefaultExtension(const nsAString & aDefaultExtension) = 0;

  /**
  * The filter which is currently selected in the File Picker dialog
  *
  * @return Returns the index (0 based) of the selected filter in the filter list. 
  */
  /* attribute long filterIndex; */
  NS_IMETHOD GetFilterIndex(PRInt32 *aFilterIndex) = 0;
  NS_IMETHOD SetFilterIndex(PRInt32 aFilterIndex) = 0;

  /**
  * Set the directory that the file open/save dialog initially displays
  *
  * @param      displayDirectory  the name of the directory
  *
  */
  /* attribute nsILocalFile displayDirectory; */
  NS_IMETHOD GetDisplayDirectory(nsILocalFile * *aDisplayDirectory) = 0;
  NS_IMETHOD SetDisplayDirectory(nsILocalFile * aDisplayDirectory) = 0;

  /**
  * Get the nsILocalFile for the file or directory.
  *
  * @return Returns the file currently selected
  */
  /* readonly attribute nsILocalFile file; */
  NS_IMETHOD GetFile(nsILocalFile * *aFile) = 0;

  /**
  * Get the nsIFileURL for the file or directory.
  *
  * @return Returns the file currently selected
  */
  /* readonly attribute nsIFileURL fileURL; */
  NS_IMETHOD GetFileURL(nsIFileURL * *aFileURL) = 0;

  /**
  * Get the enumerator for the selected files
  * only works in the modeOpenMultiple mode
  *
  * @return Returns the files currently selected
  */
  /* readonly attribute nsISimpleEnumerator files; */
  NS_IMETHOD GetFiles(nsISimpleEnumerator * *aFiles) = 0;

  /**
  * Show File Dialog. The dialog is displayed modally.
  *
  * @return returnOK if the user selects OK, returnCancel if the user selects cancel
  *
  */
  /* short show (); */
  NS_IMETHOD Show(PRInt16 *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIFILEPICKER \
  NS_IMETHOD Init(nsIDOMWindow *parent, const nsAString & title, PRInt16 mode); \
  NS_IMETHOD AppendFilters(PRInt32 filterMask); \
  NS_IMETHOD AppendFilter(const nsAString & title, const nsAString & filter); \
  NS_IMETHOD GetDefaultString(nsAString & aDefaultString); \
  NS_IMETHOD SetDefaultString(const nsAString & aDefaultString); \
  NS_IMETHOD GetDefaultExtension(nsAString & aDefaultExtension); \
  NS_IMETHOD SetDefaultExtension(const nsAString & aDefaultExtension); \
  NS_IMETHOD GetFilterIndex(PRInt32 *aFilterIndex); \
  NS_IMETHOD SetFilterIndex(PRInt32 aFilterIndex); \
  NS_IMETHOD GetDisplayDirectory(nsILocalFile * *aDisplayDirectory); \
  NS_IMETHOD SetDisplayDirectory(nsILocalFile * aDisplayDirectory); \
  NS_IMETHOD GetFile(nsILocalFile * *aFile); \
  NS_IMETHOD GetFileURL(nsIFileURL * *aFileURL); \
  NS_IMETHOD GetFiles(nsISimpleEnumerator * *aFiles); \
  NS_IMETHOD Show(PRInt16 *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIFILEPICKER(_to) \
  NS_IMETHOD Init(nsIDOMWindow *parent, const nsAString & title, PRInt16 mode) { return _to Init(parent, title, mode); } \
  NS_IMETHOD AppendFilters(PRInt32 filterMask) { return _to AppendFilters(filterMask); } \
  NS_IMETHOD AppendFilter(const nsAString & title, const nsAString & filter) { return _to AppendFilter(title, filter); } \
  NS_IMETHOD GetDefaultString(nsAString & aDefaultString) { return _to GetDefaultString(aDefaultString); } \
  NS_IMETHOD SetDefaultString(const nsAString & aDefaultString) { return _to SetDefaultString(aDefaultString); } \
  NS_IMETHOD GetDefaultExtension(nsAString & aDefaultExtension) { return _to GetDefaultExtension(aDefaultExtension); } \
  NS_IMETHOD SetDefaultExtension(const nsAString & aDefaultExtension) { return _to SetDefaultExtension(aDefaultExtension); } \
  NS_IMETHOD GetFilterIndex(PRInt32 *aFilterIndex) { return _to GetFilterIndex(aFilterIndex); } \
  NS_IMETHOD SetFilterIndex(PRInt32 aFilterIndex) { return _to SetFilterIndex(aFilterIndex); } \
  NS_IMETHOD GetDisplayDirectory(nsILocalFile * *aDisplayDirectory) { return _to GetDisplayDirectory(aDisplayDirectory); } \
  NS_IMETHOD SetDisplayDirectory(nsILocalFile * aDisplayDirectory) { return _to SetDisplayDirectory(aDisplayDirectory); } \
  NS_IMETHOD GetFile(nsILocalFile * *aFile) { return _to GetFile(aFile); } \
  NS_IMETHOD GetFileURL(nsIFileURL * *aFileURL) { return _to GetFileURL(aFileURL); } \
  NS_IMETHOD GetFiles(nsISimpleEnumerator * *aFiles) { return _to GetFiles(aFiles); } \
  NS_IMETHOD Show(PRInt16 *_retval) { return _to Show(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIFILEPICKER(_to) \
  NS_IMETHOD Init(nsIDOMWindow *parent, const nsAString & title, PRInt16 mode) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(parent, title, mode); } \
  NS_IMETHOD AppendFilters(PRInt32 filterMask) { return !_to ? NS_ERROR_NULL_POINTER : _to->AppendFilters(filterMask); } \
  NS_IMETHOD AppendFilter(const nsAString & title, const nsAString & filter) { return !_to ? NS_ERROR_NULL_POINTER : _to->AppendFilter(title, filter); } \
  NS_IMETHOD GetDefaultString(nsAString & aDefaultString) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDefaultString(aDefaultString); } \
  NS_IMETHOD SetDefaultString(const nsAString & aDefaultString) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDefaultString(aDefaultString); } \
  NS_IMETHOD GetDefaultExtension(nsAString & aDefaultExtension) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDefaultExtension(aDefaultExtension); } \
  NS_IMETHOD SetDefaultExtension(const nsAString & aDefaultExtension) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDefaultExtension(aDefaultExtension); } \
  NS_IMETHOD GetFilterIndex(PRInt32 *aFilterIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFilterIndex(aFilterIndex); } \
  NS_IMETHOD SetFilterIndex(PRInt32 aFilterIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFilterIndex(aFilterIndex); } \
  NS_IMETHOD GetDisplayDirectory(nsILocalFile * *aDisplayDirectory) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDisplayDirectory(aDisplayDirectory); } \
  NS_IMETHOD SetDisplayDirectory(nsILocalFile * aDisplayDirectory) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDisplayDirectory(aDisplayDirectory); } \
  NS_IMETHOD GetFile(nsILocalFile * *aFile) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFile(aFile); } \
  NS_IMETHOD GetFileURL(nsIFileURL * *aFileURL) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFileURL(aFileURL); } \
  NS_IMETHOD GetFiles(nsISimpleEnumerator * *aFiles) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFiles(aFiles); } \
  NS_IMETHOD Show(PRInt16 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Show(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsFilePicker : public nsIFilePicker
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFILEPICKER

  nsFilePicker();

private:
  ~nsFilePicker();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsFilePicker, nsIFilePicker)

nsFilePicker::nsFilePicker()
{
  /* member initializers and constructor code */
}

nsFilePicker::~nsFilePicker()
{
  /* destructor code */
}

/* void init (in nsIDOMWindow parent, in AString title, in short mode); */
NS_IMETHODIMP nsFilePicker::Init(nsIDOMWindow *parent, const nsAString & title, PRInt16 mode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void appendFilters (in long filterMask); */
NS_IMETHODIMP nsFilePicker::AppendFilters(PRInt32 filterMask)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void appendFilter (in AString title, in AString filter); */
NS_IMETHODIMP nsFilePicker::AppendFilter(const nsAString & title, const nsAString & filter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute AString defaultString; */
NS_IMETHODIMP nsFilePicker::GetDefaultString(nsAString & aDefaultString)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsFilePicker::SetDefaultString(const nsAString & aDefaultString)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute AString defaultExtension; */
NS_IMETHODIMP nsFilePicker::GetDefaultExtension(nsAString & aDefaultExtension)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsFilePicker::SetDefaultExtension(const nsAString & aDefaultExtension)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long filterIndex; */
NS_IMETHODIMP nsFilePicker::GetFilterIndex(PRInt32 *aFilterIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsFilePicker::SetFilterIndex(PRInt32 aFilterIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsILocalFile displayDirectory; */
NS_IMETHODIMP nsFilePicker::GetDisplayDirectory(nsILocalFile * *aDisplayDirectory)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsFilePicker::SetDisplayDirectory(nsILocalFile * aDisplayDirectory)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsILocalFile file; */
NS_IMETHODIMP nsFilePicker::GetFile(nsILocalFile * *aFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIFileURL fileURL; */
NS_IMETHODIMP nsFilePicker::GetFileURL(nsIFileURL * *aFileURL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsISimpleEnumerator files; */
NS_IMETHODIMP nsFilePicker::GetFiles(nsISimpleEnumerator * *aFiles)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* short show (); */
NS_IMETHODIMP nsFilePicker::Show(PRInt16 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIFilePicker_h__ */
