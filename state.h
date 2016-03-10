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
	State() {}

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

// Class provides functionality for checking sampling state.
class StateUtils {
public:
	// Verify topic's self status - word count sum.
	// Add statistics to topic state.
	static void CheckTopicState(
							Topic* topic, 
							unordered_map<Topic*, State>& topic_state,
							int& status_ok);

	// Verify table's self status - word count sum.
	// Add statistics to table state - each word count 
	// in the table.
	// Minus each word count from its topic state until
	// 0 to check topic's statistics from tables.
	static void CheckTableState(
							Table* table, 
							unordered_map<Table*, State>& table_state,
							unordered_map<Topic*, State>& topic_state,
							int& status_ok);
	
	// Verify document's self status - word count sum,
	// with tables' word count.
	static void CheckDocumentState(
							Document* document,
							unordered_map<Table*, State>& table_state,
							unordered_map<Topic*, State>& topic_state,
							int& status_ok);

	// Check the total status.
	static void CheckCorpusState(Corpus* corpus);
	
};
}  // namespace hdp
#endif  // STATE_H_