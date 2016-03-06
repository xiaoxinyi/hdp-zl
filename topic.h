#ifndef TOPIC_H_
#define TOPIC_H_

#include <vector>
#include <gsl/gsl_sf.h>

using namespace std;


namespace hdp {

class AllTopics;

// The Topic contains the counts of each word,
// the number of tables, corpus word number,
// lgam_word_eta - ln(gamma(w + eta)).
class Topic {
public:
	Topic(int corpus_word_no, AllTopics* all_topics_);

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

	// The count of tables pointing this topic.
	int table_count_;

	// Counts of each word assigned to this topic.
	vecotr<int> word_counts_;

	// Precomputed ln(gamma(w + eta)) for each word.
	vector<double> lgam_word_eta_; 

	// Log word posterior probability in the topic.
	vector<double> log_word_pr_;

};

// AllTopics store all the topics globally.
// This class provides functionality of 
// adding new topic, removing topic.
class AllTopics {
public:
	~AllTopics();

	// Sigleton instance of AllTopics.
	static AllTopics& GetInstance();

	int getTopics() const { return topic_ptrs_.size(); }

	void addNewTopic(int corpus_word_no);
	void removeTopic(Topic* topic);
	
	Topic* getMutableTopic(int i) {
		return topic_ptrs_[i];
	}

	int getEta() const { return eta_; }
	void setEta(const int& eta) { eta_ = eta; }

private:
	// All topics.
	vector<Topic*> topic_ptrs_;

	// Dirichlet parameter.
	double eta_; 

	// Private constructor.
	AllTopics() { }
	// Declare but no implemention.
	AllTopics(const AllTopics& from);
	AllTopics& operator=(const AllTopics& from);
};

class TopicTableUtils {
public:
	// Compute log gamma ratio for table given a topic.
	static double LogGammaRatio(Table* table,
											 				Topic* topic);
};

}  // namespace hdp
#endif  // TOPIC_H_