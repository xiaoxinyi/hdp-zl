#ifndef TOPIC_H_
#define TOPIC_H_

#include <vector>

using namespace std;


namespace hdp {

class AllTopics;

// The Topic contains the counts of each word,
// the number of tables, corpus word number,
// lgam_word_eta - ln(gamma(w + eta)).
class Topic {
public:
	Topic(int corpus_word_no, AllTopics* all_topics_);


private:
	// Corpus word number - vocabulary size.
	int corpus_word_no_;

	// The count of tables pointing this topic.
	int table_count_;

	// Counts of each word assigned to this topic.
	vecotr<int> word_counts_;

	// Precomputed ln(gamma(w + eta)) for each word.
	vector<double> lgam_word_eta_;

	// All topics.
	AllTopics* all_topics_; 

};

// AllTopics store all the topics globally.
// This class provides functionality of 
// adding new topic, removing topic.
class AllTopics {
public:
	~AllTopics();

	// Sigleton instance of AllTopics.
	static AllTopics& GetInstance();

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


}  // namespace hdp
#endif  // TOPIC_H_