#ifndef CORPUS_H_
#define CORPUS_H_

#include <string>

#include "document.h"
#include "utils.h"

namespace hdp {

// A corpus containing a number of documents.
// The parameters of two levels - gamma, alpha.
class Corpus {
 public:
  Corpus();
  Corpus(double gamma, double alpha);

  Corpus(Corpus&& from) = default;
  Corpus& operator=(Corpus&& from) = default;

  void setWordNo(int word_no) { word_no_ = word_no; }
  int getWordNo() const { return word_no_; }

  void addDocument(Document&& document) {
    documents_.emplace_back(move(document));
  }
  int getDocuments() const { return documents_.size(); }
  Document* getMutableDocument(int i) { return &documents_.at(i); }
  void setDocuments(vector<Document>&& documents) {
    documents_ = move(documents);
  }

  int getWordTotal() const { return word_total_; }
  void setWordTotal(int word_total) { word_total_ = word_total; }

  void setGamma(int gamma) { gamma_ = gamma; }
  int getGamma() const { return gamma_; }

  void setAlpha(int alpha) { alpha_ = alpha; }
  int getAlpha() const { return alpha_; }
 private:
  // Parameters of the dirichlet process.
  // gamma shows concentration of tables.
  double gamma_;

  // Parameter of the dirichlet process.
  // alpha shows how concentrated of words in the document.
  double alpha_;

  // The number of distinct words in the corpus.
  int word_no_;

  // The number of total words in the corpus.
  int word_total_;

  // The documents in this corpus.
  vector<Document> documents_;
};

// This class provides functionality for reading a corpus from a file.
class CorpusUtils {
 public:
  // Read corpus from file.
  static void ReadCorpus(
      const std::string& filename,
      Corpus* corpus);


  // Permute the documents in the corpus.
  static void PermuteDocuments(Corpus* corpus);

  // Compute Alpha score.
  static double AlphaScore(Corpus* corpus);
};

}  // namespace hdp

#endif  // CORPUS_H_
