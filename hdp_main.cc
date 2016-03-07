
#include <iostream>

#include "gibbs.h"

using hdp::GibbsSampler;
using hdp::GibbsState;

#define MAX_ITERATIONS 10000

int main(int argc, char** argv) {
  if (argc == 3) {
    // The random number generator seed.
    // For testing an example seed is: t = 1147530551;
    long rng_seed;
    (void) time(&rng_seed);

    std::string filename_corpus = argv[1];
    std::string filename_settings = argv[2];
    hdp::GibbsState* gibbs_state = hdp::GibbsSampler::InitGibbsStateRep(
        filename_corpus, filename_settings, rng_seed);

    for (int i = 0; i < MAX_ITERATIONS; i++) {
      hdp::GibbsSampler::IterateGibbsState(gibbs_state);
    }

    delete gibbs_state;
  } else {
    cout << "Arguments: "
        "(1) corpus filename "
        "(2) settings filename" << endl;
  }
  return 0;
}

