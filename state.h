#ifndef  STATE_H_
#define STATE_H_

#include <vector>
#include <unordered_map>

#include "topic.h"
#include "document.h"
#include "corpus.h"


using namespace std;

namespace hdp {

// State contains count, word_ids, word_counts.
class State {
public:
	State(int corpus_word_no, int count);
	State(int corpus_word_no);

	int getCount() const { return count_; }
	void setCount(int count) { count_ = count; }

	int getWordCount(int word_id) const { return word_counts_[word_id]; }
	void setWordCounts(const vector<int>& word_counts) {
		word_counts_ = word_counts;
	}
	void updateWordCount(int word_id, int word_count) {
		word_counts_[word_id] += word_count;
	}
	void setWordCount(int word_id, int word_count) { 
		word_counts_[word_id] = word_count;
	}
	
private:
	// Word count in table, or table count in topic.
	int count_;

	// Word count in the table or topic.
	vector<int> word_counts_;

};

class StateUtils {
public:
	static void CheckTopicState(
							Topic* topic, 
							unordered_map<Topic*, State>& topic_state);

	static void CheckTableState(
							Table* table, 
							unordered_map<Table*, State>& table_state,
							unordered_map<Topic*, State>& topic_state);
	
	static void CheckDocumentState(
							Document* document,
							unordered_map<Table*, State>& table_state,
							unordered_map<Topic*, State>& topic_state);

	static void CheckCorpusState(Corpus* corpus);
	
};
}  // namespace hdp
#endif  // STATE_H_