#include <math.h>


#include "utils.h"
#include "topic.h"

// =======================================================================
// Topic
// =======================================================================

Topic::Topic(int corpus_word_no)
		: corpus_word_no_(corpus_word_no),
		  table_count_(0),
		  word_counts_(corpus_word_no, 0) {
	AllTopics all_topics = AllTopics::GetInstance();
	double eta = all_topics.getEta();
	double lgam_w_eta = gsl_sf_lngamma(eta);

	word_counts_ = vector(corpus_word_no, 0);
	lgam_word_eta_ = vector(corpus_word_no, lgam_w_eta);

	double log_w_pr = log(1.0 / corpus_word_no);
	log_word_pr_ = vector(corpus_word_no, log_w_pr);

}

void Topic::updateWordCounts(int word_id, int update) {
	word_counts_[word_id] += update;

	double eta = AllAuthor::GetInstance().getEta();
	int word_count = word_counts_[word_id]; 
		
	lgam_word_eta_[word_id] = gsl_sf_lngamma(word_count + eta);
	log_word_pr_[word_id] = log(word_count/ (word_count + eta * corpus_word_no_ ));
}

// =======================================================================
// AllTopics
// =======================================================================

AllTopics::~AllTopics() {
	int size = topic_ptrs_.size();
	for (int i = 0; i < size; i++) {
		if (topic_ptrs_[i] != NULL) {
			delete topic_ptrs_[i];
		}
	}
}

AllTopics::AllTopics& GetInstance() {
	static AllTopics instance;
	return instance;
}
	
void AllTopics::addNewTopic(int corpus_word_no) {
	Topic* topic = new Topic(corpus_word_no);
	topic_ptrs_.push_back(topic);
}

void AllTopics::removeTopic(Topic* topic) {
	removeFromVec(topic_ptrs_, topic);
} 