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

#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <list>

#include "./ServerSocket.h"
#include "./HttpServer.h"
#include "./HttpUtils.h"

using std::cerr;
using std::cout;
using std::endl;

// Print out program usage, and exit() with EXIT_FAILURE.
void Usage(char *progname);

// Parses the command-line arguments, invokes Usage() on failure.
// "port" is a return parameter to the port number to listen on,
// "path" is a return parameter to the directory containing
// our static files, and "indices" is a return parameter to a
// list of index filenames.  Ensures that the path is a readable
// directory, and the index filenames are readable, and if not,
// invokes Usage() to exit.
void GetPortAndPath(int argc,
                    char **argv,
                    uint16_t *port,
                    std::string *path,
                    std::list<std::string> *indices);

int main(int argc, char **argv) {
  // Print out welcome message.
  cout << "Welcome to http333d, the UW cse333 web server!" << endl;
  cout << "  Copyright 2012 Steven Gribble" << endl;
  cout << "  http://www.cs.washington.edu/homes/gribble" << endl;
  cout << endl;
  cout << "initializing:" << endl;
  cout << "  parsing port number and static files directory..." << endl;

  // Ignore the SIGPIPE signal, otherwise we'll crash out if a client
  // disconnects unexpectedly.
  signal(SIGPIPE, SIG_IGN);

  // Get the port number and list of index files.
  uint16_t portnum;
  std::string staticdir;
  std::list<std::string> indices;
  GetPortAndPath(argc, argv, &portnum, &staticdir, &indices);
  cout << "    port: " << portnum << endl;
  cout << "    path: " << staticdir << endl;

  // Run the server.
  hw4::HttpServer hs(portnum, staticdir, indices);
  if (!hs.Run()) {
    cerr << "  server failed to run!?" << endl;
  }

  cout << "server completed!  Exiting." << endl;
  return EXIT_SUCCESS;
}


void Usage(char *progname) {
  cerr << "Usage: " << progname << " port staticfiles_directory indices+";
  cerr << endl;
  exit(EXIT_FAILURE);
}

void GetPortAndPath(int argc,
                    char **argv,
                    uint16_t *port,
                    std::string *path,
                    std::list<std::string> *indices) {
  // Be sure to check a few things:
  //  (a) that you have a sane number of command line arguments
  //  (b) that the port number is reasonable
  //  (c) that "path" (i.e., argv[2]) is a readable directory
  //  (d) that you have at least one index, and that all indices
  //      are readable files.

  // MISSING:

  // Check num arguements.
  if (argc < 4) {
    cout << "Error: num args" << endl;
    Usage(argv[0]);
  }

  // Parse arguments.
  // Port
  int res = strtol(argv[1], nullptr, 0);
  if (res < 0 || res > UINT16_MAX) {
    // Bad port number
    cout << "Error: port" << endl;
    Usage(argv[0]);
  }
  *port = (uint16_t)res;

  // Directory
  std::string directory(argv[2]);
  if (!hw4::IsPathSafe(std::string("/"), directory)) {
    // Bad directory name
    cout << "Error: staticfiles_directory" << endl;
    cout << directory << endl;
    Usage(argv[0]);
  }
  *path = directory;

  // Indices
  std::list<std::string> ret_indices;
  for (int i = 3; i < argc; i++) {
    std::string index(argv[i]);
    if (!hw4::IsPathSafe(std::string("/"), index)) {
      // Bad index
      cout << "Error: indices " << i << endl;
      Usage(argv[0]);
    }
    ret_indices.push_front(std::string(argv[i]));
  }
  *indices = ret_indices;
}

