#ifndef TOPIC_H_
#define TOPIC_H_

#include <vector>
#include <gsl/gsl_sf.h>

#include "document.h"

using namespace std;


namespace hdp {

// The Topic contains the counts of each word,
// the number of tables, corpus word number,
// lgam_word_eta - ln(gamma(w + eta)).
class Topic {
public:
	Topic(int corpus_word_no);

	int getTopicWordNo() const { return topic_word_no_; }
	void setTopicWordNo(int topic_word_no) { topic_word_no_ = topic_word_no; }
	void incTopicWordNo(int val) { topic_word_no_ += val; }

	int getCorpusWordNo() const { return corpus_word_no_; }
	void setCorpusWordNO(int corpus_word_no) { corpus_word_no_ = corpus_word_no; }

	int getTableCount() const { return table_count_; }
	void setTableCount(int table_count) { table_count = table_count_; }
	void incTableCount(int val) { table_count_ += val; }

	void updateWordCounts(int word_id, int update);
	int getWordCount(int word_id) const { return word_counts_[word_id]; }

	double getLgamWordEta(int word_id) const { return lgam_word_eta_[word_id]; }
	double getLogWordPr(int word_id) const { return log_word_pr_[word_id]; }

	int getTopicWords() const;
private:
	// Corpus word number - vocabulary size.
	int corpus_word_no_;

	// Word count in the topic.
	int topic_word_no_;

	// The count of tables pointing this topic.
	int table_count_;

	// Counts of each word assigned to this topic.
	vector<int> word_counts_;

	// Precomputed ln(gamma(w + eta)) for each word.
	vector<double> lgam_word_eta_; 

	// Log word posterior probability in the topic.
	vector<double> log_word_pr_;

	// Map table to word count from this table.
	// unordered_map<Table*, int> map_table_word_cout_;

};

// The class provides functionality for computing
// eta score.
class TopicUtils {
public:
	// Computing eta score for given topic.
	static double EtaScore(Topic* topic, double eta);
};

// AllTopics store all the topics globally.
// This class provides functionality of 
// adding new topic, removing topic,
// compacting topics after sampling.
class AllTopics {
public:
	~AllTopics();

	// Sigleton instance of AllTopics.
	static AllTopics& GetInstance();

	int getTopics() const { return topic_ptrs_.size(); }

	void addNewTopic(int corpus_word_no);
	void removeTopic(Topic* topic);
	void removeTopic(int pos);
	void removeLastTopic();
	void compactTopics();
	
	Topic* getMutableTopic(int i) {
		return topic_ptrs_[i];
	}

	int getEta() const { return eta_; }
	void setEta(const int& eta) { eta_ = eta; }

private:
	// All topics.
	vector<Topic*> topic_ptrs_;

	// Dirichlet parameter for word in topic.
	double eta_; 

	// Private constructor.
	AllTopics() { }
	// Declare but no implemention.
	AllTopics(const AllTopics& from);
	AllTopics& operator=(const AllTopics& from);
};

// This class provides functionality for computing
// Log gamma ratio of each topic given a table.
class TopicTableUtils {
public:
	// Compute log gamma ratio for table given a topic.
	// table - given table.
	// topic - possible to be sampled topic.
	// word_ids - distinct word ids in table.
	// counts - corresponding counts.
	static double LogGammaRatio(Table* table,
											 				Topic* topic,
											 				vector<int>& word_ids,
											 				vector<int>& counts);
};

// This class provides functionality for computing
// Gamma score, Eta scores.
class AllTopicsUtils {
public:
	// Compute gamma score.
	// gamma - first level dirichlet process parameter.
	static double GammaScore(double gamma);

	// Compute eta score.
	// eta - dirichlet distribution parameter of each topic.
	static double EtaScore();
};

}  // namespace hdp
#endif  // TOPIC_H_