//========================================================================
//
// GHash.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include "goo/gmem.h"
#include "goo/GString.h"
#include "goo/GHash.h"

//------------------------------------------------------------------------

struct GHashBucket {
  GString *key;
  union {
    void *p;
    int i;
  } val;
  GHashBucket *next;
};

struct GHashIter {
  int h;
  GHashBucket *p;
};

//------------------------------------------------------------------------

GHash::GHash(GBool deleteKeysA) {
  int h;

  deleteKeys = deleteKeysA;
  size = 7;
  tab = (GHashBucket **)gmallocn(size, sizeof(GHashBucket *));
  for (h = 0; h < size; ++h) {
    tab[h] = NULL;
  }
  len = 0;
}

GHash::~GHash() {
  GHashBucket *p;
  int h;

  for (h = 0; h < size; ++h) {
    while (tab[h]) {
      p = tab[h];
      tab[h] = p->next;
      if (deleteKeys) {
	delete p->key;
      }
      delete p;
    }
  }
  gfree(tab);
}

void GHash::add(GString *key, void *val) {
  GHashBucket *p;
  int h;

  // expand the table if necessary
  if (len >= size) {
    expand();
  }

  // add the new symbol
  p = new GHashBucket;
  p->key = key;
  p->val.p = val;
  h = hash(key);
  p->next = tab[h];
  tab[h] = p;
  ++len;
}

void GHash::add(const GString *key, void *val) {
  if (deleteKeys) {
    fprintf(stderr, "BUG in GHash::add with const key and deleteKeys");
    exit(1);
  }
  add((GString *)key, val);
}

void GHash::add(GString *key, int val) {
  GHashBucket *p;
  int h;

  // expand the table if necessary
  if (len >= size) {
    expand();
  }

  // add the new symbol
  p = new GHashBucket;
  p->key = key;
  p->val.i = val;
  h = hash(key);
  p->next = tab[h];
  tab[h] = p;
  ++len;
}

void GHash::add(const GString *key, int val) {
  if (deleteKeys) {
    fprintf(stderr, "BUG in GHash::add with const key and deleteKeys");
    exit(1);
  }
  add((GString*)key, val);
}

void GHash::replace(GString *key, void *val, GBool delKey) {
  GHashBucket *p;
  int h;

  if ((p = find(key, &h))) {
    p->val.p = val;
    if (delKey)
      delete key;
  } else {
    add(key, val);
  }
}

void GHash::replace(GString *key, void *val) {
  replace(key, val, gTrue);
}

void GHash::replace(const GString *key, void *val) {
  if (deleteKeys) {
    fprintf(stderr, "BUG in GHash::replace with const key and deleteKeys");
    exit(1);
  }
  replace((GString *)key, val, gFalse);
}

void GHash::replace(GString *key, int val, GBool delKey) {
  GHashBucket *p;
  int h;

  if ((p = find(key, &h))) {
    p->val.i = val;
    if (delKey)
      delete key;
  } else {
    add(key, val);
  }
}

void GHash::replace(GString *key, int val) {
  replace(key, val, gTrue);
}

void GHash::replace(const GString *key, int val) {
  if (deleteKeys) {
    fprintf(stderr, "BUG in GHash::replace with const key and deleteKeys");
    exit(1);
  }
  replace((GString *)key, val, gFalse);
}

void *GHash::lookup(const GString *key)const {
  GHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  return p->val.p;
}

int GHash::lookupInt(const GString *key)const {
  GHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  return p->val.i;
}

void *GHash::lookup(const char *key)const {
  GHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  return p->val.p;
}

int GHash::lookupInt(const char *key)const {
  GHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  return p->val.i;
}

void *GHash::remove(const GString *key) {
  GHashBucket *p;
  GHashBucket **q;
  void *val;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.p;
  delete p;
  --len;
  return val;
}

int GHash::removeInt(const GString *key) {
  GHashBucket *p;
  GHashBucket **q;
  int val;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.i;
  delete p;
  --len;
  return val;
}

void *GHash::remove(const char *key) {
  GHashBucket *p;
  GHashBucket **q;
  void *val;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.p;
  delete p;
  --len;
  return val;
}

int GHash::removeInt(const char *key) {
  GHashBucket *p;
  GHashBucket **q;
  int val;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.i;
  delete p;
  --len;
  return val;
}

void GHash::startIter(GHashIter **iter) {
  *iter = new GHashIter;
  (*iter)->h = -1;
  (*iter)->p = NULL;
}

GBool GHash::getNext(GHashIter **iter, GString **key, void **val) {
  if (!*iter) {
    return gFalse;
  }
  if ((*iter)->p) {
    (*iter)->p = (*iter)->p->next;
  }
  while (!(*iter)->p) {
    if (++(*iter)->h == size) {
      delete *iter;
      *iter = NULL;
      return gFalse;
    }
    (*iter)->p = tab[(*iter)->h];
  }
  *key = (*iter)->p->key;
  *val = (*iter)->p->val.p;
  return gTrue;
}

GBool GHash::getNext(GHashIter **iter, GString **key, int *val) {
  if (!*iter) {
    return gFalse;
  }
  if ((*iter)->p) {
    (*iter)->p = (*iter)->p->next;
  }
  while (!(*iter)->p) {
    if (++(*iter)->h == size) {
      delete *iter;
      *iter = NULL;
      return gFalse;
    }
    (*iter)->p = tab[(*iter)->h];
  }
  *key = (*iter)->p->key;
  *val = (*iter)->p->val.i;
  return gTrue;
}

void GHash::killIter(GHashIter **iter) {
  delete *iter;
  *iter = NULL;
}

void GHash::expand() {
  GHashBucket **oldTab;
  GHashBucket *p;
  int oldSize, h, i;

  oldSize = size;
  oldTab = tab;
  size = 2*size + 1;
  tab = (GHashBucket **)gmallocn(size, sizeof(GHashBucket *));
  for (h = 0; h < size; ++h) {
    tab[h] = NULL;
  }
  for (i = 0; i < oldSize; ++i) {
    while (oldTab[i]) {
      p = oldTab[i];
      oldTab[i] = oldTab[i]->next;
      h = hash(p->key);
      p->next = tab[h];
      tab[h] = p;
    }
  }
  gfree(oldTab);
}

GHashBucket *GHash::find(const GString *key, int *h)const {
  GHashBucket *p;

  *h = hash(key);
  for (p = tab[*h]; p; p = p->next) {
    if (!p->key->cmp(key)) {
      return p;
    }
  }
  return NULL;
}

GHashBucket *GHash::find(const char *key, int *h)const {
  GHashBucket *p;

  *h = hash(key);
  for (p = tab[*h]; p; p = p->next) {
    if (!p->key->cmp(key)) {
      return p;
    }
  }
  return NULL;
}

int GHash::hash(const GString *key)const {
  char *p;
  unsigned int h;
  int i;

  h = 0;
  for (p = key->getCString(), i = 0; i < key->getLength(); ++p, ++i) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}

int GHash::hash(const char *key)const {
  const char *p;
  unsigned int h;

  h = 0;
  for (p = key; *p; ++p) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}
