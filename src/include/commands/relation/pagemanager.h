#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include "ipagemanager.h"
#include "page.h"

void initializePage(
      void*         self,
	  void*         page, 
	  size_t        size, 
	  size_t        suplSize);

size_t getFreeSpace(
	  void*         self,
	  void*         page);

uint16 addItemToPage(
	  void*          page,
      void*          item,
      size_t         size,
	  uint16         itemnum,
      Bool           overwrite);

#endif