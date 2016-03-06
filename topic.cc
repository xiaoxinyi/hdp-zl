#include <math.h>
#include <assert.h>

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
	log_word_pr_[word_id] = log(word_count) -log(word_count + eta * corpus_word_no_ );
}

int Topic::getTopicWords() const {
	int sum = 0;
	for (auto count : word_counts_) {
		sum += count;
	}

	return sum; 
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

// =======================================================================
// TopicTableUtils
// =======================================================================

double TopicTableUtils::LogGammaRatio(Table* table,
											 								Topic* topic) {
	double log_gamma_ratio = 0.0;
	Topic* old_topic = table->getMutableTopic();

	double eta = AllTopics::GetInstance().getEta();
	int topic_words = topic->getTopicWords();
	int table_words = table->getWordCount();
	int corpus_word_no = topic->getCorpusWordNo();
	vector<int> word_ids;
	vector<int> counts;
	TableUtils::GetWordsAndCounts(table, word_ids, counts);

	
	if (old_topic == topic) {
		log_gamma_ratio += gsl_sf_lngamma(corpus_word_no * eta + topic_words - table_words) -
										 gsl_sf_lngamma(corpus_word_no * eta + topic_words)

		for (int i = 0; i < (int)word_ids.size(); i++) {
			int word_id = word_ids[i];
			int word_count = topic->getWordCount(word_id);
			log_gamma_ratio += topic->getLgamWordEta(word_id) -
												 gsl_sf_lngamma(word_count + eta - counts[i]); 
		}
	} else {
		log_gamma_ratio += gsl_sf_lngamma(corpus_word_no * eta + topic_words) -
										 gsl_sf_lngamma(corpus_word_no * eta + topic_words + table_words)

		for (int i = 0; i < (int)word_ids.size(); i++) {
			int word_id = word_ids[i];
			int word_count = topic->getWordCount(word_id);
			log_gamma_ratio += gsl_sf_lngamma(word_count + eta + counts[i]) - 
												 topic->getLgamWordEta(word_id);  
		}
	}
	
	return log_gamma_ratio;
}