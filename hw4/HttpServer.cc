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

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

#include "./FileReader.h"
#include "./HttpConnection.h"
#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpServer.h"
#include "./libhw3/QueryProcessor.h"

using std::cerr;
using std::cout;
using std::endl;

namespace hw4 {

// This is the function that threads are dispatched into
// in order to process new client connections.
void HttpServer_ThrFn(ThreadPool::Task *t);

// Given a request, produce a response.
HttpResponse ProcessRequest(const HttpRequest &req,
                            const std::string &basedir,
                            const std::list<std::string> *indices);

// Process a file request.
HttpResponse ProcessFileRequest(const std::string &uri,
                                const std::string &basedir);

// Process a query request.
HttpResponse ProcessQueryRequest(const std::string &uri,
                                 const std::list<std::string> *indices);

bool HttpServer::Run(void) {
  // Create the server listening socket.
  int listen_fd;
  cout << "  creating and binding the listening socket..." << endl;
  if (!ss_.BindAndListen(AF_INET6, &listen_fd)) {
    cerr << endl << "Couldn't bind to the listening socket." << endl;
    return false;
  }

  // Spin, accepting connections and dispatching them.  Use a
  // threadpool to dispatch connections into their own thread.
  cout << "  accepting connections..." << endl << endl;
  ThreadPool tp(kNumThreads);
  while (1) {
    HttpServerTask *hst = new HttpServerTask(HttpServer_ThrFn);
    hst->basedir = staticfile_dirpath_;
    hst->indices = &indices_;
    if (!ss_.Accept(&hst->client_fd,
                    &hst->caddr,
                    &hst->cport,
                    &hst->cdns,
                    &hst->saddr,
                    &hst->sdns)) {
      // The accept failed for some reason, so quit out of the server.
      // (Will happen when kill command is used to shut down the server.)
      break;
    }
    // The accept succeeded; dispatch it.
    tp.Dispatch(hst);
  }
  return true;
}

void HttpServer_ThrFn(ThreadPool::Task *t) {
  // Cast back our HttpServerTask structure with all of our new
  // client's information in it.
  std::unique_ptr<HttpServerTask> hst(static_cast<HttpServerTask *>(t));
  cout << "  client " << hst->cdns << ":" << hst->cport << " "
       << "(IP address " << hst->caddr << ")" << " connected." << endl;

  bool done = false;
  // Placed here to avoid deleting data
  // left in hc's buffer.
  HttpConnection hc(hst->client_fd);
  while (!done) {
    // Use the HttpConnection class to read in the next request from
    // this client, process it by invoking ProcessRequest(), and then
    // use the HttpConnection class to write the response.  If the
    // client sent a "Connection: close\r\n" header, then shut down
    // the connection.

    // MISSING:

    HttpRequest req;
    HttpResponse resp;
    hc.GetNextRequest(&req);

    if (req.headers["connection"] == "close") {
      // Client has closed connection
      done = true;
    }

    if (req.URI.length() != 0) {
      // Client sent another request. Process and respond.
      resp = ProcessRequest(req, hst->basedir, hst->indices);
      hc.WriteResponse(resp);
    }
  }
}

HttpResponse ProcessRequest(const HttpRequest &req,
                            const std::string &basedir,
                            const std::list<std::string> *indices) {
  // Is the user asking for a static file?
  if (req.URI.substr(0, 8) == "/static/") {
    return ProcessFileRequest(req.URI, basedir);
  }

  // The user must be asking for a query.
  return ProcessQueryRequest(req.URI, indices);
}


HttpResponse ProcessFileRequest(const std::string &uri,
                                const std::string &basedir) {
  // The response we'll build up.
  HttpResponse ret;

  // Steps to follow:
  //  - use the URLParser class to figure out what filename
  //    the user is asking for.
  //
  //  - use the FileReader class to read the file into memory
  //
  //  - copy the file content into the ret.body
  //
  //  - depending on the file name suffix, set the response
  //    Content-type header as appropriate, e.g.,:
  //      --> for ".html" or ".htm", set to "text/html"
  //      --> for ".jpeg" or ".jpg", set to "image/jpeg"
  //      --> for ".png", set to "image/png"
  //      etc.
  //
  // be sure to set the response code, protocol, and message
  // in the HttpResponse as well.
  std::string fname = "";

  // MISSING:

  // Get filename
  URLParser parser;
  parser.Parse(uri);
  fname = parser.get_path();
  fname.erase(0, 7);  // Delete "/static"

  // Read in the file
  FileReader reader(basedir, fname);
  string file;
  bool res = reader.ReadFile(&file);

  // Add file extension
  string extension = fname.substr(fname.find_last_of("."), string::npos);
  pair<string, string> filetype;
  if (extension.compare(string(".html")) == 0) {
      filetype = pair<string, string>("Content-type", "text/html");
  } else if (extension.compare(string(".htm")) == 0) {
      filetype = pair<string, string>("Content-type", "text/html");
  } else if (extension.compare(string(".jpeg")) == 0) {
      filetype = pair<string, string>("Content-type", "image/jpeg");
  } else if (extension.compare(string(".jpg")) == 0) {
      filetype = pair<string, string>("Content-type", "image/jpeg");
  } else if (extension.compare(string(".png")) == 0) {
      filetype = pair<string, string>("Content-type", "image/png");
  } else if (extension.compare(string(".css")) == 0) {
      filetype = pair<string, string>("Content-type", "text/css");
  } else if (extension.compare(string(".js")) == 0) {
      filetype = pair<string, string>("Content-type", "application/javascript");
  } else if (extension.compare(string(".xml")) == 0) {
      filetype = pair<string, string>("Content-type", "text/xml");
  } else if (extension.compare(string(".ico")) == 0) {
      filetype = pair<string, string>("Content-type", "text/ico");
  }

  if (res && filetype.second != "ERROR") {
    // Successfully read file! Format into HTTP response.
    ret.protocol = "HTTP/1.1";
    ret.response_code = 200;
    ret.message = "OK";
    ret.headers.insert(filetype);
    ret.body = file;
    return ret;
  }


  // If you couldn't find the file, return an HTTP 404 error.
  ret.protocol = "HTTP/1.1";
  ret.response_code = 404;
  ret.message = "Not Found";
  ret.body = "<html><body>Couldn't find file \"";
  ret.body +=  EscapeHTML(fname);
  ret.body += "\"</body></html>";
  return ret;
}

HttpResponse ProcessQueryRequest(const std::string &uri,
                                 const std::list<std::string> *indices) {
  // The response we're building up.
  HttpResponse ret;

  // Your job here is to figure out how to present the user with
  // the same query interface as our solution_binaries/http333d server.
  // A couple of notes:
  //
  //  - no matter what, you need to present the 333gle logo and the
  //    search box/button
  //
  //  - if the user had previously typed in a search query, you also
  //    need to display the search results.
  //
  //  - you'll want to use the URLParser to parse the uri and extract
  //    search terms from a typed-in search query.  convert them
  //    to lower case.
  //
  //  - you'll want to create and use a hw3::QueryProcessor to process
  //    the query against the search indices
  //
  //  - in your generated search results, see if you can figure out
  //    how to hyperlink results to the file contents, like we did
  //    in our solution_binaries/http333d.

  // MISSING:

  // Decode request
  hw4::URLParser parser;
  parser.Parse(uri);
  string path = parser.get_path();

  if (path.compare("/query") != 0 && path.compare("/") != 0) {
    // Resource does not exist. Respond with 404
    ret.protocol = "HTTP/1.1";
    ret.response_code = 404;
    ret.message = "Not Found";
    ret.body = "<html><body>Couldn't find file \"";
    ret.body +=  EscapeHTML(path);
    ret.body += "\"</body></html>";
    return ret;
  }

  // Get query arguments
  string raw_query = parser.get_args()["terms"];
  vector<string> query;
  boost::split(query, raw_query, boost::is_any_of("+"));

  // Convert query to lowercase
  for (auto it = query.begin(); it != query.end(); it++) {
    boost::to_lower(*it);
  }

  // Process the query
  hw3::QueryProcessor qp(*indices, true);
  vector<hw3::QueryProcessor::QueryResult> results =
        qp.ProcessQuery(query);

  // Build up responce body.
  string body;

  // Copy/paste in its full glory.
  body += "<html><head><title>333gle</title></head><body><center style=\"";
  body += "font-size:500%;\"><span style=\"position:relative;bottom:-0.33em;";
  body += "color:orange;\">3</span><span style=\"color:red;\">3</span><span";
  body += " style=\"color:gold;\">3</span><span style=\"color:blue;\">g</span>";
  body += "<span style=\"color:green;\">l</span><span style=\"color:red;\">";
  body += "e</span></center><p><div style=\"height:20px;\"></div><center>";
  body += "<form action=\"/query\" method=\"get\"><input type=\"text\"";
  body += "size=30 name=\"terms\" /><input type=\"submit\" value=\"Search\"";
  body += "/></form></center>";

  if (results.size() > 0) {
    // Add results
    body += "<p><p><br>";
    body += std::to_string(results.size());
    body += " results found for <b>";
    for (auto it = query.begin(); it != query.end(); it++) {
      // Add search terms.
      body += *it;
      body += " ";
    }
    body += "</b><p><ul>";
    for (auto it = results.begin(); it != results.end(); it++) {
      // Add results.
      body += "<li><a href=\"/static/";
      body += it->document_name;
      body += "\">";
      body += it->document_name;
      body += "</a> [";
      body += std::to_string(it->rank);
      body += "]<br>";
    }
    body += "</ul>";
  } else if (path.compare("/") != 0) {
    body += "<p><p><br> No results found for <b>";
    for (auto it = query.begin(); it != query.end(); it++) {
      // Add search terms.
      body += *it;
      body += " ";
    }
    body += "</b><p>";
  }

  body += "</body></html>";

  // Build up the response.
  ret.protocol = "HTTP/1.1";
  ret.response_code = 200;
  ret.message = "OK";
  ret.body += body;

  return ret;
}

}  // namespace hw4
