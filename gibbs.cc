

#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "gibbs.h"
#include "document.h"
#include "state.h"

#define REP_NO 1
#define DEFAULT_HYPER_LAG 0
#define DEFAULT_SHUFFLE_LAG 100
#define DEFAULT_SAMPLE_GAMMA 0
#define DEFAULT_SAMPLE_ETA 0
#define DEFUALT_SAMPLE_ALPHA 0
#define BUF_SIZE 100
#define CHECK 1
#define DEBUG 0

namespace hdp {

// =======================================================================
// GibbsState
// =======================================================================

GibbsState::GibbsState()
    : score_(0.0),
      alpha_score_(0.0),
      eta_score_(0.0),
      gamma_score_(0.0),
      max_score_(0.0),
      iteration_(0),
      shuffle_lag_(DEFAULT_SHUFFLE_LAG),
      hyper_lag_(DEFAULT_HYPER_LAG),
      sample_eta_(DEFAULT_SAMPLE_ETA),
      sample_alpha_(DEFUALT_SAMPLE_ALPHA),
      sample_gamma_(DEFAULT_SAMPLE_GAMMA) {
}



double GibbsState::computeGibbsScore() {
  // Compute the Alpha, Eta and Gamma scores.
  alpha_score_ = CorpusUtils::AlphaScore(&corpus_);
  eta_score_ = AllTopicsUtils::EtaScore();
  gamma_score_ = AllTopicsUtils::GammaScore(corpus_.getGamma());
  score_ = alpha_score_ + eta_score_ + gamma_score_;
  cout << "Alpha_score: " << alpha_score_ << endl;
  cout << "Eta_score: " << eta_score_ << endl;
  cout << "Gamma_score: " << gamma_score_ << endl;
  cout << "Score: " << score_ << endl;

  // Update the maximum score if necessary.
  if (score_ > max_score_ || iteration_ == 0) {
    max_score_ = score_;
  }

  return score_;
}

// =======================================================================
// GibbsUtils
// =======================================================================

void GibbsSampler::ReadGibbsInput(
    GibbsState* gibbs_state,
    const std::string& filename_corpus,
    const std::string& filename_settings) {
  // Read hyperparameters from file
  ifstream infile(filename_settings.c_str());
  char buf[BUF_SIZE];

  int sample_eta = DEFAULT_SAMPLE_ETA, 
  		sample_alpha = DEFUALT_SAMPLE_ALPHA, 
  		sample_gamma = DEFAULT_SAMPLE_GAMMA;
  
  double alpha, gamma, eta;

  while (infile.getline(buf, BUF_SIZE)) {
    istringstream s_line(buf);
    // Consider each line at a time.
    std::string str;
    getline(s_line, str, ' ');
    std::string value;
    getline(s_line, value, ' ');
    if (str.compare("ETA") == 0) {
      eta= atof(value.c_str());
    } else if (str.compare("ALPHA") == 0) {
      alpha = atof(value.c_str());
    } else if (str.compare("GAMMA") == 0) {
      gamma = atof(value.c_str());
    } else if (str.compare("SAMPLE_ALPHA") == 0) {
      sample_alpha = atoi(value.c_str());
    } else if (str.compare("SAMPLE_ETA") == 0) {
      sample_eta = atoi(value.c_str());
    } else if (str.compare("SAMPLE_GAMMA") == 0) {
      sample_gamma = atoi(value.c_str());
    }
  }

  infile.close();

  // Create corpus.
  Corpus corpus(gamma, alpha);
  CorpusUtils::ReadCorpus(filename_corpus, &corpus);

  gibbs_state->setSampleEta(sample_eta);
  gibbs_state->setSampleAlpha(sample_alpha);
  gibbs_state->setSampleGamma(sample_gamma);
  gibbs_state->setCorpus(move(corpus));

  AllTopics::GetInstance().setEta(eta);

}

void GibbsSampler::InitGibbsState(
    GibbsState* gibbs_state) {

  Corpus* corpus = gibbs_state->getMutableCorpus();
  
  double alpha = corpus->getAlpha();
  double gamma = corpus->getGamma();

  int corpus_word_no = corpus->getWordNo();
  
  // Permute documents in the corpus.
  CorpusUtils::PermuteDocuments(corpus);

  for (int i = 0; i < corpus->getDocuments(); i++) {
    Document* document = corpus->getMutableDocument(i);

    // permute_words = 1, remove = false.
    DocumentUtils::SampleTables(document, 1, false, 
    														alpha, gamma, corpus_word_no);
    DocumentUtils::CompactTables(document);

    DocumentUtils::SampleTopics(document, gamma, false);

    AllTopics::GetInstance().compactTopics();

	}

  // Compute the Gibbs score.
  double gibbs_score = gibbs_state->computeGibbsScore();

  cout << "Gibbs score = " << gibbs_score << endl;
}

GibbsState* GibbsSampler::InitGibbsStateRep(
    const std::string& filename_corpus,
    const std::string& filename_settings,
    long random_seed) {
  double best_score = 0.0;
  GibbsState* best_gibbs_state = NULL;

  for (int i = 0; i < REP_NO; i++) {
    // Initialize the random number generator.
    Utils::InitRandomNumberGen(random_seed);

    GibbsState* gibbs_state = new GibbsState();
    ReadGibbsInput(gibbs_state, filename_corpus, filename_settings);

    // Initialize the Gibbs state.
    InitGibbsState(gibbs_state);

    // Update Gibbs best state if necessary.
    if (gibbs_state->getScore()  > best_score || i == 0) {
      if (best_gibbs_state != NULL) {
        delete best_gibbs_state;
      }
      best_gibbs_state = gibbs_state;
      best_score = gibbs_state->getScore();
      cout << "Best initial state at iteration: " <<
          i << " score " << best_score << endl;
    } else {
      delete gibbs_state;
    }
  }

  return best_gibbs_state;
}

void GibbsSampler::IterateGibbsState(GibbsState* gibbs_state) {
  assert(gibbs_state != NULL);

  Corpus* corpus = gibbs_state->getMutableCorpus();
  gibbs_state->incIteration(1);
  int current_iteration = gibbs_state->getIteration();

  if (gibbs_state->getIteration() == 100) {
  	cout << endl;
  }
  cout << "Start iteration..." << gibbs_state->getIteration() << endl;

  // Determine value for permute.
  int permute = 0;
  int shuffle_lag = gibbs_state->getShuffleLag();
  if (shuffle_lag > 0) {
    permute = 1 - (current_iteration % shuffle_lag);
  }

  if (CHECK == 1) {
  	StateUtils::CheckCorpusState(corpus);
  }

  // Permute documents in corpus.
  if (permute == 1) {
    CorpusUtils::PermuteDocuments(corpus);
  }

  double alpha = corpus->getAlpha();
  double gamma = corpus->getGamma();

  int corpus_word_no = corpus->getWordNo();

  // Sample document path and word levels.
  for (int i = 0; i < corpus->getDocuments(); i++) {
    Document* document = corpus->getMutableDocument(i);

    DocumentUtils::SampleTables(document, permute, true,
																alpha, gamma, corpus_word_no);
    DocumentUtils::CompactTables(document);

    if (DEBUG == 1) {
    	cout << "sampling tables ..." << endl;
    	DocumentUtils::PrintDocumentInfo(document);
    	AllTopicsUtils::PrintTopicsInfo();
    }
    

    DocumentUtils::SampleTopics(document, gamma, true);

    AllTopics::GetInstance().compactTopics();

    if (DEBUG == 1) {
    	cout << "sampling topics ..." << endl;
    	DocumentUtils::PrintDocumentInfo(document);
    	AllTopicsUtils::PrintTopicsInfo();
    }
    
  }

  // Sample hyper-parameters.
  if (gibbs_state->getHyperLag() > 0 &&
      (current_iteration % gibbs_state->getHyperLag() == 0)) {
    if (gibbs_state->getSampleEta() == 1) {
      // TODO 
    }
    if (gibbs_state->getSampleAlpha() == 1) {
      // TODO
    }
    if (gibbs_state->getSampleGamma() == 1) {
    	// TODO
    }
  }

  // Compute the Gibbs score with the new parameter values.
  double gibbs_score = gibbs_state->computeGibbsScore();

  cout << "Gibbs score at iteration "
       << gibbs_state->getIteration() << " = " << gibbs_score << endl;
}

}  // namespace hdp


