#include <assert.h>

#include <iostream>

#include "state.h"

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
		  word_counts_(corpus_word_no, 0) {

}

// =======================================================================
// StateUtils
// =======================================================================

void StateUtils::CheckTopicState(
								Topic* topic, 
								unordered_map<Topic*, State>& topic_state,
								int& status_ok) {
	assert(topic != NULL);

	int corpus_word_no = topic->getCorpusWordNo();
	int count = topic->getTableCount();
	int word_no = topic->getTopicWords();

	State state(corpus_word_no, count);

	// Add statistics to topic state
	int check_count = 0;
	for (int i = 0; i < corpus_word_no; i++) {
		int word_count = topic->getWordCount(i);
		state.setWordCount(i, word_count);
		check_count += word_count;
	}

	topic_state[topic] = state;

	// Verify total word_no in the topic.
	if (word_no != check_count) {
		cout << "topic self status bad!!!" << endl;
		status_ok = 0;
	} else {
		cout << "topic self status ok!" << endl;
	}
}

void StateUtils::CheckTableState(
											Table* table, 
											unordered_map<Table*, State>& table_state,
											unordered_map<Topic*, State>& topic_state,
											int& status_ok) {
	assert(table != NULL);
	Topic* topic = table->getMutableTopic();

	auto found = topic_state.find(topic);
	if (found == topic_state.end()) {
		cout << "Cannot find table's topic, status bad!!!" << endl;
		status_ok = 0;
	} 
	State& state_to_minus = topic_state[topic];

	int corpus_word_no = topic->getCorpusWordNo();
	int count = table->getWordCount();
	State state(corpus_word_no, count);
	
	// Compute word_count from table's map_word_count.
	auto& map_word_count = table->getMapWordCount();
	int check_count = 0;
	for (auto& p : map_word_count) {
		check_count += p.second;
	}
	
	// Verify word_count in the table.
	if (check_count == count) {
		cout << "table self status ok!" << endl;
		status_ok = 0;
	} else {
		cout << "table self status bad!!!" << endl;
		status_ok = 0;
	}

	// Add statistics to table state,
	// Minus statistics from coresponding topic.
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
										unordered_map<Topic*, State>& topic_state,
										int& status_ok) {
	
	int check_words = 0;

	// Add statistics to table state,
	// Minus statistics from its topic state.
	int tables = document->getTables();
	for (int i = 0; i < tables; i++) {
		Table* table = document->getMutableTable(i);
		check_words += table->getWordCount();
		CheckTableState(table, table_state, topic_state, status_ok);
	}

	int words = document->getWords();
	if (words == check_words) {
		cout << "document self status ok!" << endl;
	} else if (words > check_words) {
		cout << "table words in less than document, bad status!!" << endl; 
		status_ok = 0;
	} else {
		cout << "table words in more than document, bad status!!" << endl; 
		status_ok = 0;
	}

	// Minus statistics from table state.
	for (int i = 0; i < words; i++) {
		Word* word = document->getMutableWord(i);
		int word_id = word->getId();
		Table* table = word->getMutableTable();
		auto found = table_state.find(table);
		if (found == table_state.end()) {
			cout << "word " << word->getId() << 
							" has a bad table pointer!!!" << endl;
			status_ok = 0;
		}
		State& table_minus_state = table_state[table];
		table_minus_state.updateWordCount(word_id, -1);
	}
	
	int corpus_word_no = AllTopics::GetInstance().getMutableTopic(0)->getCorpusWordNo();

	// Verify table's statistics is 0.
	bool flag = true;
	for (auto&p : table_state) {
		State& st = p.second;
		
		for (int i = 0; i < corpus_word_no; i++) {
			if (st.getWordCount(i) != 0) {
				cout << "document words not match tables" << endl;
				flag = false;
				status_ok = 0;
			}

			if (i == corpus_word_no - 1 && flag) {
				cout << "document words match tables" << endl;
			}
		}
	}

	return;
}

void StateUtils::CheckCorpusState(Corpus* corpus) {
	unordered_map<Table*, State> table_state;
	unordered_map<Topic*, State> topic_state;
	int status_ok = 1;

	AllTopics& all_topics = AllTopics::GetInstance();
	int topics = all_topics.getTopics();

	for (int i = 0; i < topics; i++) {
		cout << "CHECK TOPIC " << i << " : " << endl;
		Topic* topic = all_topics.getMutableTopic(i);
		CheckTopicState(topic, topic_state, status_ok);
	}

	int docs = corpus->getDocuments();
	for (int i = 0; i < docs; i++) {
		// Each document has a table state.
		table_state.clear();
		Document* document = corpus->getMutableDocument(i);
		cout << "CHECK DOCUMENT " << i << " : " << endl;
		CheckDocumentState(document, table_state, topic_state, status_ok);
	}

	int corpus_word_no = all_topics.getMutableTopic(0)->getCorpusWordNo();
	bool flag = true;
	for (auto& p : topic_state) {
		State& s = p.second;
		
		for (int i = 0; i < corpus_word_no; i++) {
			if (s.getWordCount(i) != 0) {
				cout << "tables words not match topic." << endl;
				flag = false;
				status_ok = 0;
			}

			if (i == corpus_word_no - 1 && flag) {
				cout << "tables words match topic." << endl;
			}
		}
	}

	if (status_ok == 1) {
		cout << "STATUS OK!!!" << endl;
	} else {
		cout << "STSTUS BAD!!!" << endl;
	}
	
}




}  // namespace hdp
