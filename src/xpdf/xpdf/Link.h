//========================================================================
//
// Link.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef LINK_H
#define LINK_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "xpdf/Object.h"

class GString;
class Array;
class Dict;

//------------------------------------------------------------------------
// LinkAction
//------------------------------------------------------------------------

enum LinkActionKind {
  actionGoTo,			// go to destination
  actionGoToR,			// go to destination in new file
  actionLaunch,			// launch app (or open document)
  actionURI,			// URI
  actionNamed,			// named action
  actionMovie,			// movie action
  actionUnknown			// anything else
};

class LinkAction {
public:

  // Destructor.
  virtual ~LinkAction() {}

  // Was the LinkAction created successfully?
  virtual GBool isOk()const = 0;

  // Check link action type.
  virtual LinkActionKind getKind()const = 0;

  // Parse a destination (old-style action) name, string, or array.
  static LinkAction *parseDest(const Object *obj);

  // Parse an action dictionary.
  static LinkAction *parseAction(const Object *obj, const GString *baseURI = NULL);

  // Extract a file name from a file specification (string or
  // dictionary).
  static GString *getFileSpecName(const Object *fileSpecObj);
};

//------------------------------------------------------------------------
// LinkDest
//------------------------------------------------------------------------

enum LinkDestKind {
  destXYZ,
  destFit,
  destFitH,
  destFitV,
  destFitR,
  destFitB,
  destFitBH,
  destFitBV
};

class LinkDest {
public:

  // Build a LinkDest from the array.
  LinkDest(const Array *a);

  // Copy a LinkDest.
  LinkDest *copy()const { return new LinkDest(this); }

  // Was the LinkDest created successfully?
  GBool isOk()const { return ok; }

  // Accessors.
  LinkDestKind getKind()const { return kind; }
  GBool isPageRef()const { return pageIsRef; }
  int getPageNum()const { return pageNum; }
  const Ref& getPageRef()const { return pageRef; }
  double getLeft()const { return left; }
  double getBottom()const { return bottom; }
  double getRight()const { return right; }
  double getTop()const { return top; }
  double getZoom()const { return zoom; }
  GBool getChangeLeft()const { return changeLeft; }
  GBool getChangeTop()const { return changeTop; }
  GBool getChangeZoom()const { return changeZoom; }

private:

  LinkDestKind kind;		// destination type
  GBool pageIsRef;		// is the page a reference or number?
  union {
    Ref pageRef;		// reference to page
    int pageNum;		// one-relative page number
  };
  double left, bottom;		// position
  double right, top;
  double zoom;			// zoom factor
  GBool changeLeft, changeTop;	// for destXYZ links, which position
  GBool changeZoom;		//   components to change
  GBool ok;			// set if created successfully

  LinkDest(const LinkDest *dest);
};

//------------------------------------------------------------------------
// LinkGoTo
//------------------------------------------------------------------------

class LinkGoTo: public LinkAction {
public:

  // Build a LinkGoTo from a destination (dictionary, name, or string).
  LinkGoTo(const Object *destObj);

  // Destructor.
  virtual ~LinkGoTo();

  // Was the LinkGoTo created successfully?
  virtual GBool isOk()const { return dest || namedDest; }

  // Accessors.
  virtual LinkActionKind getKind()const { return actionGoTo; }
  const LinkDest *getDest()const { return dest; }
  const GString *getNamedDest()const { return namedDest; }

private:

  LinkDest *dest;		// regular destination (NULL for remote
				//   link with bad destination)
  GString *namedDest;		// named destination (only one of dest and
				//   and namedDest may be non-NULL)
};

//------------------------------------------------------------------------
// LinkGoToR
//------------------------------------------------------------------------

class LinkGoToR: public LinkAction {
public:

  // Build a LinkGoToR from a file spec (dictionary) and destination
  // (dictionary, name, or string).
  LinkGoToR(const Object *fileSpecObj, const Object *destObj);

  // Destructor.
  virtual ~LinkGoToR();

  // Was the LinkGoToR created successfully?
  virtual GBool isOk()const { return fileName && (dest || namedDest); }

  // Accessors.
  virtual LinkActionKind getKind()const { return actionGoToR; }
  const GString *getFileName()const { return fileName; }
  const LinkDest *getDest()const { return dest; }
  const GString *getNamedDest()const { return namedDest; }

private:

  GString *fileName;		// file name
  LinkDest *dest;		// regular destination (NULL for remote
				//   link with bad destination)
  GString *namedDest;		// named destination (only one of dest and
				//   and namedDest may be non-NULL)
};

//------------------------------------------------------------------------
// LinkLaunch
//------------------------------------------------------------------------

class LinkLaunch: public LinkAction {
public:

  // Build a LinkLaunch from an action dictionary.
  LinkLaunch(const Object *actionObj);

  // Destructor.
  virtual ~LinkLaunch();

  // Was the LinkLaunch created successfully?
  virtual GBool isOk()const { return fileName != NULL; }

  // Accessors.
  virtual LinkActionKind getKind()const { return actionLaunch; }
  const GString *getFileName()const { return fileName; }
  const GString *getParams()const { return params; }

private:

  GString *fileName;		// file name
  GString *params;		// parameters
};

//------------------------------------------------------------------------
// LinkURI
//------------------------------------------------------------------------

class LinkURI: public LinkAction {
public:

  // Build a LinkURI given the URI (string) and base URI.
  LinkURI(const Object *uriObj, const GString *baseURI);

  // Destructor.
  virtual ~LinkURI();

  // Was the LinkURI created successfully?
  virtual GBool isOk()const { return uri != NULL; }

  // Accessors.
  virtual LinkActionKind getKind()const { return actionURI; }
  const GString *getURI()const { return uri; }

private:

  GString *uri;			// the URI
};

//------------------------------------------------------------------------
// LinkNamed
//------------------------------------------------------------------------

class LinkNamed: public LinkAction {
public:

  // Build a LinkNamed given the action name.
  LinkNamed(const Object *nameObj);

  virtual ~LinkNamed();

  virtual GBool isOk()const { return name != NULL; }

  virtual LinkActionKind getKind()const { return actionNamed; }
  const GString *getName()const { return name; }

private:

  GString *name;
};

//------------------------------------------------------------------------
// LinkMovie
//------------------------------------------------------------------------

class LinkMovie: public LinkAction {
public:

  LinkMovie(const Object *annotObj, const Object *titleObj);

  virtual ~LinkMovie();

  virtual GBool isOk()const { return annotRef.num >= 0 || title != NULL; }

  virtual LinkActionKind getKind()const { return actionMovie; }
  const GBool hasAnnotRef()const { return annotRef.num >= 0; }
  const Ref *getAnnotRef()const { return &annotRef; }
  const GString *getTitle()const { return title; }

private:

  Ref annotRef;
  GString *title;
};

//------------------------------------------------------------------------
// LinkUnknown
//------------------------------------------------------------------------

class LinkUnknown: public LinkAction {
public:

  // Build a LinkUnknown with the specified action type.
  LinkUnknown(const char *actionA);

  // Destructor.
  virtual ~LinkUnknown();

  // Was the LinkUnknown create successfully?
  virtual GBool isOk()const { return action != NULL; }

  // Accessors.
  virtual LinkActionKind getKind()const { return actionUnknown; }
  const GString *getAction()const { return action; }

private:

  GString *action;		// action subtype
};

//------------------------------------------------------------------------
// Link
//------------------------------------------------------------------------

class Link {
public:

  // Construct a link, given its dictionary.
  Link(const Dict *dict, const GString *baseURI);

  // Destructor.
  ~Link();

  // Was the link created successfully?
  GBool isOk()const { return ok; }

  // Check if point is inside the link rectangle.
  GBool inRect(double x, double y)const
    { return x1 <= x && x <= x2 && y1 <= y && y <= y2; }

  // Get action.
  const LinkAction *getAction()const { return action; }

  // Get the link rectangle.
  void getRect(double *xa1, double *ya1, double *xa2, double *ya2)const
    { *xa1 = x1; *ya1 = y1; *xa2 = x2; *ya2 = y2; }

private:

  double x1, y1;		// lower left corner
  double x2, y2;		// upper right corner
  LinkAction *action;		// action
  GBool ok;			// is link valid?
};

//------------------------------------------------------------------------
// Links
//------------------------------------------------------------------------

class Links {
public:

  // Extract links from array of annotations.
  Links(const Object *annots, const GString *baseURI);

  // Destructor.
  ~Links();

  // Iterate through list of links.
  int getNumLinks()const { return numLinks; }
  const Link *getLink(int i)const { return links[i]; }

  // If point <x>,<y> is in a link, return the associated action;
  // else return NULL.
  const LinkAction *find(double x, double y)const;

  // Return true if <x>,<y> is in a link.
  GBool onLink(double x, double y)const;

private:

  Link **links;
  int numLinks;
};

#endif
