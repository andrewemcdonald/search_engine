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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <stdlib.h>

#include "./QueryProcessor.h"

using namespace std;

static void Usage(char *progname) {
  std::cerr << "Usage: " << progname << " [index files+]" << std::endl;
  exit(EXIT_FAILURE);
}

// Parses a line of input text (from cin) and returns a vector of tokens 
// delimited by whitespace. Returns an empty vector if EOF was reached.
static vector<string> getUserInput();

// Your job is to implement the entire filesearchshell.cc
// functionality. We're essentially giving you a blank screen to work
// with; you need to figure out an appropriate design, to decompose
// the problem into multiple functions or classes if that will help,
// to pick good interfaces to those functions/classes, and to make
// sure that you don't leak any memory.
//
// Here are the requirements for a working solution:
//
// The user must be able to run the program using a command like:
//
//   ./filesearchshell ./foo.idx ./bar/baz.idx /tmp/blah.idx [etc]
//
// i.e., to pass a set of filenames of indices as command line
// arguments. Then, your program needs to implement a loop where
// each loop iteration it:
//
//  (a) prints to the console a prompt telling the user to input the
//      next query.
//
//  (b) reads a white-space separated list of query words from
//      std::cin, converts them to lowercase, and constructs
//      a vector of c++ strings out of them.
//
//  (c) uses QueryProcessor.cc/.h's QueryProcessor class to
//      process the query against the indices and get back a set of
//      query results.  Note that you should instantiate a single
//      QueryProcessor  object for the lifetime of the program, rather
//      than  instantiating a new one for every query.
//
//  (d) print the query results to std::cout in the format shown in
//      the transcript on the hw3 web page.
//
// Also, you're required to quit out of the loop when std::cin
// experiences EOF, which a user passes by pressing "control-D"
// on the console.  As well, users should be able to type in an
// arbitrarily long query -- you shouldn't assume anything about
// a maximum line length.  Finally, when you break out of the
// loop and quit the program, you need to make sure you deallocate
// all dynamically allocated memory.  We will be running valgrind
// on your filesearchshell implementation to verify there are no
// leaks or errors.
//
// You might find the following technique useful, but you aren't
// required to use it if you have a different way of getting the
// job done.  To split a std::string into a vector of words, you
// can use a std::stringstream to get the job done and the ">>"
// operator. See, for example, "gnomed"'s post on stackoverflow for
// his example on how to do this:
//
//   http://stackoverflow.com/questions/236129/c-how-to-split-a-string
//
// (Search for "gnomed" on that page.  He uses an istringstream, but
// a stringstream gets the job done too.)
//
// Good luck, and write beautiful code!
int main(int argc, char **argv) {
  if (argc < 2) Usage(argv[0]);

  // Build list of folders to be indexed
  list<string> idxfolds;
  for (int i = 1; i < argc; i++) {
    string idxfold(argv[i]);
    idxfolds.push_back(idxfold);
  }

  // Build query processor.
  hw3::QueryProcessor qp(idxfolds);

  while (1) {
    // Request and parse user input.
    cout << "Enter input:" << endl;
    vector<string> input = getUserInput();
    if (input.size() == 0) {
      // EOF reached; terminate program.
      return EXIT_SUCCESS;
    }

    // Convert all words tolowercase
    for (auto it = input.begin(); it != input.end(); it++) {
      transform(it->begin(), it->end(), it->begin(), ::tolower);
    }

    // Process the requested query.
    vector<hw3::QueryProcessor::QueryResult> results = qp.ProcessQuery(input);

    // Print results.
    for (auto it = results.begin(); it != results.end(); it++) {
      cout << it->document_name << " (" << it->rank << ")" << endl;
    }
  }

  return EXIT_SUCCESS;
}

static vector<string> getUserInput() {
  string inputline;
  vector<string> tokens;

  // Read input into a stringstream.
  if (!getline(cin, inputline)) {
    // Reached EOF
    return tokens;
  }
  stringstream ss(inputline);

  // Insert into vector.
  copy(istream_iterator<string>(ss),
       istream_iterator<string>(),
       back_inserter(tokens));

  return tokens;
}

