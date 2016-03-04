#include "utils.h"
#include "topic.h"

// =======================================================================
// Topic
// =======================================================================

Topic::Topic(int corpus_word_no, AllTopics* all_topics)
		: corpus_word_no_(corpus_word_no),
		  table_count_(0),
		  word_counts_(corpus_word_no, 0),
		  all_topics_(all_topics) {
	double eta = all_topics_.getEta();
	double lgam_w_eta = gsl_sf_lngamma(eta);
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