/*
 * Copyright ©2019 Justin Hsia.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2019 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "memindex.h"
#include "filecrawler.h"

#define INIT_BUF_SIZE 128

static void Usage(void);

// Reads user input into a buffer until
// EOF/return character reached or memory error.
// Dynamically resizes buffer as needed.
//
// Exits the program if there is not enough memory
// to store the user request.
static void getUserInput(char* buf, size_t size);

// Parses the user input, performs query on
// a MemIndex, and returns results.
//
// Parameters:
//
// - query : The query to preocess.
//
// - index : The inverted index to perform the query on.
//
// Returns:
//
// - A LinkedList of search results for the given
// query iff successful.
//
// - NULL if the query did not match any documents or was otherwise
// unsuccessful.
static LinkedList processQuery(char* query, MemIndex index);

// Splits a string into tokens delimited by the
// ' ' character and interts a '\0' at then end
// of each token.
//
// Parameters:
// - str : The string to split. Will be modified by split().
// - numElements : Return parameter that contains the number
//                 of tokens created by split()
//
// Returns: An array of char*s, where each pointer refers to the
// next token in the string. The last pointer will
// always be NULL.
static char **split(char *str, int *numElements);

int main(int argc, char **argv) {
  if (argc != 2)
    Usage();

  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - crawl from a directory provided by argv[1] to produce and index
  //  - prompt the user for a query and read the query from stdin, in a loop
  //  - split a query into words (check out strtok_r)
  //  - process a query against the index and print out the results
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.

  // IMPLEMENTATION ERROR!!! DID NOT CONSIDER CAPITALIZED INPUT CASE

  size_t bufSize = INIT_BUF_SIZE;
  char *buf = (char*)malloc(INIT_BUF_SIZE);
  int res;
  LinkedList ll;
  LLIter lliter;
  SearchResultPtr sr;
  MemIndex index;
  DocTable docTab;
  char *docName;

  // Build inverted index.
  printf("Indexing '%s'\n", argv[1]);
  res = CrawlFileTree(argv[1], &docTab, &index);

  if (res != 1) {
    printf("Error: Failed to crawl \"%s\"\n", argv[1]);
    return EXIT_FAILURE;
  }

  printf("enter query:\n");
  getUserInput(buf, bufSize);
  while (*buf != EOF) {
    // Continuously process queries from user.
    ll = processQuery(buf, index);

    if (ll != NULL) {
      // Query successful.
      lliter = LLMakeIterator(ll, 0);
      while (LLIteratorHasNext(lliter)) {
        LLIteratorGetPayload(lliter, (LLPayload_t *)&sr);
        docName = DTLookupDocID(docTab, sr->docid);
        printf("%s (%d)\n", docName, sr->rank);
        LLIteratorDelete(lliter, (LLPayloadFreeFnPtr)&free);
      }
      LLIteratorGetPayload(lliter, (LLPayload_t *)&sr);
      docName = DTLookupDocID(docTab, sr->docid);
      printf("%s (%d)\n", docName, sr->rank);
      LLIteratorDelete(lliter, (LLPayloadFreeFnPtr)&free);
    }
    LLIteratorFree(lliter);
    FreeLinkedList(ll, (LLPayloadFreeFnPtr)&free);
    printf("enter query:\n");
    getUserInput(buf, bufSize);
  }

  free(buf);
  FreeMemIndex(index);
  FreeDocTable(docTab);
  return EXIT_SUCCESS;
}

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot>\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
  exit(EXIT_FAILURE);
}


static void getUserInput(char* buf, size_t size) {
  int index = -1;

  do {
    // Read a byte of data
    if (size == ++index) {
      // Resize buffer
      buf = realloc(buf, size += INIT_BUF_SIZE);
      Verify333(buf != NULL);
    }

    buf[index] = (char)fgetc(stdin);
  }  while (EOF != buf[index] && '\n' != buf[index]);

  // Null-terminate input string.
  if (buf[index] == EOF) {
    buf[0] = EOF;
  } else {
    buf[index] = '\0';
  }
}

static LinkedList processQuery(char *query, MemIndex index) {
  int numElements;
  char **words = split(query, &numElements);
  LinkedList ret = MIProcessQuery(index, words, numElements);
  free(words);
  return ret;
}

static char **split(char *str, int *numElements) {
  size_t size = INIT_BUF_SIZE;
  char **ret = (char**)malloc(INIT_BUF_SIZE);
  int count = 0;
  char* reentrant;

  // First case, strtok requires a string a input.
  ret[count] = strtok_r(str, " ", &reentrant);
  if (size /  8 <= ++count) {
    ret = realloc(ret, size += INIT_BUF_SIZE);
    Verify333(ret != NULL);
  }

  while (NULL != (ret[count] = strtok_r(NULL, " ", &reentrant))) {
    // Increment count and add buffer space if needed.
    if (size /  8 <= ++count) {
      ret = realloc(ret, size += INIT_BUF_SIZE);
      Verify333(ret != NULL);
    }
  }

  *numElements = count;
  return ret;
}
