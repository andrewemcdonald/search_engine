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

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

#define BUF_SIZE 1024

using std::map;
using std::string;
using std::vector;


namespace hw4 {

bool HttpConnection::GetNextRequest(HttpRequest *request) {
  // Use "WrappedRead" to read data into the buffer_
  // instance variable.  Keep reading data until either the
  // connection drops or you see a "\r\n\r\n" that demarcates
  // the end of the request header.
  //
  // Once you've seen the request header, use ParseRequest()
  // to parse the header into the *request argument.
  //
  // Very tricky part:  clients can send back-to-back requests
  // on the same socket.  So, you need to preserve everything
  // after the "\r\n\r\n" in buffer_ for the next time the
  // caller invokes GetNextRequest()!

  // MISSING:

  // Intermediate buffer to avoid potential internal overflow
  // issues with string class.
  char buf[BUF_SIZE + 1];
  buf[BUF_SIZE] = '\0';

  while (1) {
    // Read in the next request.

    // Check if more bytes need to be read.
    if (buffer_.find("\r\n\r\n") != string::npos) {
      break;
    }

    // If needed, read more bytes into buffer_.
    int res = WrappedRead(fd_, (unsigned char *) buf, BUF_SIZE);
    if (res == -1) {
      // Read failed
      return false;
    }
    buffer_.append(buf);
  }

  // Parse most recent request.
  int end = buffer_.find("\r\n\r\n") + 4;
  *request = ParseRequest(end);

  // Truncate the front of buffer_ to the beggining of the next request.
  buffer_ = buffer_.erase(0, end);

  return true;
}

bool HttpConnection::WriteResponse(const HttpResponse &response) {
  std::string str = response.GenerateResponseString();
  int res = WrappedWrite(fd_,
                         (unsigned char *) str.c_str(),
                         str.length());
  if (res != static_cast<int>(str.length()))
    return false;
  return true;
}

HttpRequest HttpConnection::ParseRequest(size_t end) {
  HttpRequest req;
  req.URI = "/";  // by default, get "/".

  // Get the header.
  std::string str = buffer_.substr(0, end);

  // Split the header into lines.  Extract the URI from the first line
  // and store it in req.URI.  For each additional line beyond the
  // first, extract out the header name and value and store them in
  // req.headers (i.e., req.headers[headername] = headervalue).
  // You should look at HttpResponse.h for details about the HTTP header
  // format that you need to parse.
  //
  // You'll probably want to look up boost functions for (a) splitting
  // a string into lines on a "\r\n" delimiter, (b) trimming
  // whitespace from the end of a string, and (c) converting a string
  // to lowercase.

  // MISSING:

  // Separate header into a vector of delimiter-separated tokens.
  vector<string> tokens;
  boost::split(tokens, str, boost::is_any_of("\r\n"));

  // Extract URI
  std::vector<string> firstline_elements;
  boost::split(firstline_elements, tokens[0], boost::is_any_of(" "));
  req.URI = firstline_elements[1];

  for (auto it = ++tokens.begin(); it != tokens.end(); it++) {
    // Add each token into req

    // Skip empty strings (result of parsing two delimiters)
    if ((*it).length() == 0) {
      // empty
      continue;
    }

    // Parse header line
    std::vector<string> hf_pair_split;
    boost::split(hf_pair_split, *it, boost::is_any_of(":"));

    // Prepare header/field data
    boost::to_lower(hf_pair_split[0]);
    boost::trim(hf_pair_split[1]);

    // Insert in to map
    pair<string, string> hf_pair(hf_pair_split[0], hf_pair_split[1]);
    req.headers.insert(hf_pair);
  }

  return req;
}

}  // namespace hw4
