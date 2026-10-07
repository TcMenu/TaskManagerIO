#ifndef SIMPLE_COLLECTIONS_H
#define SIMPLE_COLLECTIONS_H

// compiler warning telling the user to update their includes
#if defined(__GNUC__) || defined(__clang__)
#warning "SimpleCollections.h is deprecated and moved to TaskManagerIO. Please include <BTreeList.h> directly."
#elif defined(_MSC_VER)
#pragma message("WARNING: SimpleCollections.h is deprecated and moved to TaskManagerIO. Please include <BTreeList.h> directly.")
#endif

#include "BTreeList.h" // or the new header path inside TaskManagerIO

#endif
