/*
 * Copyright ©2019 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2019 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <iostream>
#include <algorithm>
#include <map>
#include <utility>

#include "./QueryProcessor.h"

extern "C" {
  #include "./libhw1/CSE333.h"
}

namespace hw3 {

QueryProcessor::QueryProcessor(list<string> indexlist, bool validate) {
  // Stash away a copy of the index list.
  indexlist_ = indexlist;
  arraylen_ = indexlist_.size();
  Verify333(arraylen_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader *[arraylen_];
  itr_array_ = new IndexTableReader *[arraylen_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::iterator idx_iterator = indexlist_.begin();
  for (HWSize_t i = 0; i < arraylen_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = new DocTableReader(fir.GetDocTableReader());
    itr_array_[i] = new IndexTableReader(fir.GetIndexTableReader());
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (HWSize_t i = 0; i < arraylen_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string> &query) {
  Verify333(query.size() > 0);
  vector<QueryProcessor::QueryResult> finalresult;

  // MISSING:
  
  // For the first keyword, go through every index and get the docids for
  // every result in that index. Then convert in to query_results and
  // insert into finalresult.
  for (HWSize_t j = 0; j < arraylen_; j++) {
    // Get docid map
    DocIDTableReader *ditr = itr_array_[j]->LookupWord(query[0]);
    if (ditr == nullptr) {
      // No results, go to next index.
      continue;
    }

    // Get list of docid_element_headers
    list<docid_element_header> kwres = ditr->GetDocIDList();

    // Convert each element into a list of query results
    // and add in to master list.
    for (auto it = kwres.begin(); it != kwres.end(); it++) {
      docid_element_header element = *it;
      QueryProcessor::QueryResult q;


      // Lookup docid in the corresponding doctable, and add result to q
      dtr_array_[j]->LookupDocID(element.docid, &q.document_name);

      // Add chain_len as the rank field of q
      q.rank = element.num_positions;

      // Insert q into finalresult
      finalresult.push_back(q);
    }
    delete ditr;
  }

  // Repeat process for any additional keywords, except this time
  // instead of simply inserting into finalresult, insert query
  // result data into a map, then iterate through finalresult
  // and look for matches.
  std::map<string, HWSize_t> matches;
  auto k = query.begin();
  for (k++; k != query.end(); k++) {
    string keyword = *k;

    // Get map of docname->rank for the next keyword
    for (HWSize_t j = 0; j < arraylen_; j++) {
      // Get index docid map
      DocIDTableReader *ditr = itr_array_[j]->LookupWord(keyword);
      if (ditr == nullptr) {
        // No results, go to next index.
        continue;
      }

      // Get list of docid_element_headers
      list<docid_element_header> kwres = ditr->GetDocIDList();

      // Add the data of each element into the map.
      for (auto it = kwres.begin(); it != kwres.end(); it++) {
        docid_element_header element = *it;

        // Lookup docid in the corresponding doctable
        string docname;
        dtr_array_[j]->LookupDocID(element.docid, &docname);

        // Insert into the map
        matches.insert(std::pair<string, HWSize_t>(docname, element.num_positions));
      }
      delete ditr;
    }

    // Iterate through finalresult, checking for matches
    for (auto it = finalresult.begin();
         it != finalresult.end(); it++) {

      // Ask the map if the docname was a result of keyword query
      if (matches.find(it->document_name) != matches.end()) {
        // Yes, query was a result; update docname rank
        std::pair<string, HTKey_t> kv = *(matches.find(it->document_name));
        it->rank += kv.second;
      } else {
        // No, query was not a result; remove entry from finalresult
        it = finalresult.erase(it);
        it--;
      }
    }
    matches.clear();  
  }

  // Sort the final results.
  std::sort(finalresult.begin(), finalresult.end());


  return finalresult;
}


}  // namespace hw3
