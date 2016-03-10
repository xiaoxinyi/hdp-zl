#include <assert.h>

#include <iostream>

namespace hdp {

// =======================================================================
// State
// =======================================================================

State::State(int corpus_word_no, int count)
		: count_(count),
		  word_counts_(corpus_word_no, 0) {

}

State::State(int corpus_word_no) 
		: count_(0),
		: word_counts_(corpus_word_no, 0) {

}

// =======================================================================
// StateUtils
// =======================================================================

void StateUtils::CheckTopicState(
								Topic* topic, 
								unordered_map<Topic*, State>& topic_state) {
	assert(topic != NULL);

	int corpus_word_no = topic->getCorpusWordNo();
	int count = topic->getTableCount();
	int word_no = topic->getWordNo();

	State state(corpus_word_no, count);
	int check_count = 0;
	for (int i = 0; i < corpus_word_no; i++) {
		int word_count = topic->getWordCount(i);
		state.setWordCount(i, word_count);
		check_count += word_count;
	}

	topic_state[topic] = state;
	if (word_no != check_count) {
		cout << "topic self status bad!!!" << endl;
	} else {
		cout << "topic self status ok!" << endl;
	}
}

void StateUtils::CheckTableState(
											Table* table, 
											unordered_map<Table*, State>& table_state,
											unordered_map<Topic*, State>& topic_state) {
	assert(table != NULL);
	Topic* topic = table->getMutableTopic();

	auto found = topic_state.find(topic);
	if (found == topic_state.end()) {
		cout << "Cannot find table's topic, status bad!!!" << endl;
	} 
	State& state_to_minus = topic_state[topic];

	int corpus_word_no = topic->getCorpusWordNo();
	inrt count = table->getWordCount();
	State state(count, corpus_word_no);

	if (table->getMapSize() == count) {
		cout << "table self status ok!" << endl;
	} else {
		cout << "table self status bad!!!" << endl;
	}

	for (int i = 0; i < corpus_word_no; i++) {
		int word_count = table->getWordCount(i);
		state.setWordCount(i, word_count);
		state_to_minus.updateWordCount(i, -1 * word_count);
	}

	table_state[table] = state;

}

void StateUtils::CheckDocumentState(
										Document* document,
										unordered_map<Table*, State>& table_state,
										unordered_map<Topic*, State>& topic_state) {
	int words = document->getwords();
	int check_words = 0;
	for (int i = 0; i < words; i++) {
		Word* word = document->getMutableWord(i);
		int word_id = word->getId();
		Table* table = word->getMutableTable()
		auto found = table_state.find(table);
		if (found == table_state.end()) {
			cout << "word " << word->getId() << 
							" has a bad table pointer!!!" << endl;
		}
		State& table_minus_state = table_state[table];
		table_minus_state.updateWordCount(word_id, -1);
	}

	for (auto& p : talbe_state) {
		Table* table = p.first;
		State& ts = p.second;
		
	}
}



}  // namespace hdp
