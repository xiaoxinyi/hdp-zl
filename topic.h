#ifndef TOPIC_H_
#define TOPIC_H_

#include <vector>

using namespace std;


namespace hdp {

// The Topic contains the counts of each word,
// the number of tables, corpus word number,
// lgam_word_eta - ln(gamma(w + eta)).
class Topic {
public:
	Topic(int corpus_word_no);

private:
	// The count of tables pointing this topic.
	int table_count_;

	// Corpus word number - vocabulary size.
	int corpus_word_no_;

	// Counts of each word assigned to this topic.
	vecotr<int> word_counts_;

	// Precomputed ln(gamma(w + eta)) for each word.
	vector<double> lgam_word_eta_;

	static vector<Topic*> AllTopics;
};


}  // namespace hdp
#endif  // TOPIC_H_