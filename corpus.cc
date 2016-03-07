// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include <assert.h>
#include <gsl/gsl_permutation.h>
#include <math.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "corpus.h"


#define BUF_SIZE 10000

namespace hdp {

// =======================================================================
// Corpus
// =======================================================================

Corpus::Corpus()
    : gamma_(0.0),
      alpha_(0.0),
      word_no_(0),
      word_total_(0) {
}

Corpus::Corpus(double gamma, double alpha)
    : gamma_(gamma),
      alpha_(alpha),
      word_no_(0), 
      word_total_(0) {
}


// =======================================================================
// CorpusUtils
// =======================================================================

void CorpusUtils::ReadCorpus(
    const std::string& filename,
    Corpus* corpus) {

  ifstream infile(filename.c_str());
  char buf[BUF_SIZE];

  int doc_no = 0;
  int word_no = 0;
  int total_word_count = 0;
  int words;

  while (infile.getline(buf, BUF_SIZE)) {
    istringstream s_line(buf);
    // Consider each line at a time.
    int word_count_pos = 0;
    Document document(doc_no, depth);
    while (s_line.getline(buf, BUF_SIZE, ' ')) {
      if (word_count_pos == 0) {
        words = atoi(buf);
      } else {
        int word_id, word_count;
        istringstream s_word_count(buf);
        std::string str;
        getline(s_word_count, str, ':');
        word_id = atoi(str.c_str());
        getline(s_word_count, str, ':');
        word_count = atoi(str.c_str());
        total_word_count += word_count;
        for (int i = 0; i < word_count; i++) {
        	document.addWord(word_id);        	
        }
        if (word_id >= word_no) {
          word_no = word_id + 1;
        }
      }
      word_count_pos++;
    }
    corpus->addDocument(document);
    doc_no += 1;
  }

  infile.close();

  corpus->setWordNo(word_no);
  corpus->setWordTotal(total_word_count);
  cout << "Number of documents in corpus: " << doc_no << endl;
  cout << "Number of distinct words in corpus: " << word_no << endl;
  cout << "Number of words in corpus: " << total_word_count << endl;
}



void CorpusUtils::PermuteDocuments(Corpus* corpus) {
  int size = corpus->getDocuments();
  vector<Document> permuted_documents;

  // Permute the values in perm.
  // These values correspond to the indices of the documents in the
  // document vector of the corpus.
  gsl_permutation* perm = gsl_permutation_calloc(size);
  Utils::Shuffle(perm, size);
  int perm_size = perm->size;
  assert(size == perm_size);

  for (int i = 0; i < perm_size; i++) {
    permuted_documents.push_back(*corpus->getMutableDocument(perm->data[i]));
  }

  corpus->setDocuments(permuted_documents);

  gsl_permutation_free(perm);
}

double CorpusUtils::AlphaScore(Corpus* corpus) {
	double alpha = corpus->getAlpha();
	int doc_no = corpus->getDocuments();
	double score = 0.0;
	for (int i = 0; i < doc_no; i++) {
		Document* document = corpus->getMutableDocument(i);
		score += DocumentUtils::AlphaScore(document, alpha);
	}

	return score;
}

}  // namespace hdp




