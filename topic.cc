#include <math.h>
#include <assert.h>

#include "utils.h"
#include "topic.h"

namespace hdp {
// =======================================================================
// Topic
// =======================================================================

Topic::Topic(int corpus_word_no)
		: corpus_word_no_(corpus_word_no),
			topic_word_no_(0),
		  table_count_(0),
		  word_counts_(corpus_word_no, 0) {
	AllTopics& all_topics = AllTopics::GetInstance();
	double eta = all_topics.getEta();
	double lgam_w_eta = gsl_sf_lngamma(eta);

	word_counts_ = vector<int>(corpus_word_no, 0);
	lgam_word_eta_ = vector<double>(corpus_word_no, lgam_w_eta);

	double log_w_pr = log(1.0 / corpus_word_no);
	log_word_pr_ = vector<double>(corpus_word_no, log_w_pr);

}

void Topic::updateWordCounts(int word_id, int update) {
	word_counts_[word_id] += update;

	double eta = AllTopics::GetInstance().getEta();
	int word_count = word_counts_[word_id]; 
		
	lgam_word_eta_[word_id] = gsl_sf_lngamma(word_count + eta);
	double log_w_pr = log(eta) - log(topic_word_no_ + corpus_word_no_ * eta);
	for (int i = 0; i < corpus_word_no_; i++) {
		if (word_counts_[i] == 0) {
			log_word_pr_[i] = log_w_pr;
		} else {
			log_word_pr_[i] = log(word_counts_[i] + eta) -
												log(topic_word_no_ + eta * corpus_word_no_);
		}
	}
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

AllTopics& AllTopics::GetInstance() {
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

void AllTopics::removeTopic(int pos) {
	auto it = topic_ptrs_.begin() + pos;
	Topic* topic = *it;
	assert(topic->getTableCount() == 0);

	delete *it;
	topic = NULL;
	*it = NULL;
	topic_ptrs_.erase(it);
}


void AllTopics::removeLastTopic() {
	int size = topic_ptrs_.size();
	delete topic_ptrs_[size];
	topic_ptrs_[size] = NULL;
	topic_ptrs_.resize(size - 1);
}

void AllTopics::compactTopics() {
	int size = topic_ptrs_.size();
	for (int i = size - 1; i >=0; --i) {
		if (topic_ptrs_[i]->getTableCount() == 0) {
			removeTopic(i);
		}
	}
}

// =======================================================================
// TopicUtils
// =======================================================================

double TopicUtils::EtaScore(Topic* topic, double eta) {
	double score = 0.0;
	double lgam_eta = gsl_sf_lngamma(eta);
	int corpus_word_no = topic->getCorpusWordNo();
	int topic_words = 0;
	
	score += gsl_sf_lngamma(corpus_word_no * eta);
	for (int i = 0; i < corpus_word_no; i++) {
		int word_count = topic->getWordCount(i);
		if (word_count > 0) {
			topic_words += word_count;
			score += gsl_sf_lngamma(word_count + eta) - lgam_eta;
		}
	}
	score -= gsl_sf_lngamma(corpus_word_no * eta + topic_words);

	return score;
}

// =======================================================================
// TopicTableUtils
// =======================================================================

double TopicTableUtils::LogGammaRatio(Table* table,
											 								Topic* topic,
											 								vector<int>& word_ids,
											 								vector<int>& counts) {
	double log_gamma_ratio = 0.0;
	Topic* old_topic = table->getMutableTopic();

	double eta = AllTopics::GetInstance().getEta();
	int topic_words = topic->getTopicWords();
	int table_words = table->getWordCount();
	int corpus_word_no = topic->getCorpusWordNo();


	
	if (old_topic == topic) {
		log_gamma_ratio += gsl_sf_lngamma(corpus_word_no * eta + topic_words - table_words) -
										 gsl_sf_lngamma(corpus_word_no * eta + topic_words);

		for (int i = 0; i < (int)word_ids.size(); i++) {
			int word_id = word_ids[i];
			int word_count = topic->getWordCount(word_id);
			log_gamma_ratio += topic->getLgamWordEta(word_id) -
												 gsl_sf_lngamma(word_count + eta - counts[i]); 
		}
	} else {
		log_gamma_ratio += gsl_sf_lngamma(corpus_word_no * eta + topic_words) -
										 gsl_sf_lngamma(corpus_word_no * eta + topic_words + table_words);

		for (int i = 0; i < (int)word_ids.size(); i++) {
			int word_id = word_ids[i];
			int word_count = topic->getWordCount(word_id);
			log_gamma_ratio += gsl_sf_lngamma(word_count + eta + counts[i]) - 
												 topic->getLgamWordEta(word_id);  
		}
	}

	return log_gamma_ratio;
}


// =======================================================================
// AllTopicsUtils 
// =======================================================================

double AllTopicsUtils::GammaScore(double gamma) {
	double score = 0.0;
	AllTopics& all_topics = AllTopics::GetInstance();
	int topics = all_topics.getTopics();
	int table_total = 0;

	for (int i = 0; i < topics; i++) {
		Topic* topic = all_topics.getMutableTopic(i);
		int tables = topic->getTableCount();
		score += gsl_sf_lngamma(tables);
		table_total += tables; 
	}

	score += topics * log(gamma) +
					 gsl_sf_lngamma(gamma) - 
					 gsl_sf_lngamma(table_total + gamma);

	return score;
}

double AllTopicsUtils::EtaScore() {
	AllTopics& all_topics = AllTopics::GetInstance();
	int topics = all_topics.getTopics();
	double eta = all_topics.getEta();

	double score = 0.0;
	for (int i = 0; i < topics; i++) {
		Topic* topic = all_topics.getMutableTopic(i);
		score += TopicUtils::EtaScore(topic, eta);
	}

	return score;
}

}  // namespace hdp
