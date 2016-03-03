#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <vector>

#include "topic.h"

using namespace std;

namespace hdp {

class Table; 

// Word has a id, a table assignment
class Word {
public:
	Word(int id);
	~Word();

	int getId() const { return id_; }
	void setId(const int& id) { id_ = id; }

	Table* getMutableTable() { return table_; }
	void setTable(Table* table) { table_ = table; }


 
private:
	// Word id.
	int id_;

	// A pointer to table the word is assigned to.
	Table* table_;
};

// This class has number of words in a table.
// Topic assignment of the table.
class Table {
public:
	Table();
	Table(word_count);

	int getWordCount() const { return word_count_; }
	void setWordCount(const int& word_count) { word_count_ = word_count; }

	Topic* getMuableTopic() { return topic; }
	void setTopic(Topic* topic) { topic_ = topic; }

private:
	// Number of words.
	int word_count_;

	// A pointer to topoic the table is assigned to.
	Topic* topic_;
};

// The Document contains an id, a number of words.
// A number of table pointers.
class Document {
public:
	Document(int id);
	~Document();

	int getWords() { return words_.size(); }
	int getTables() { return tables_.size(); }
	
	void addWord(const Word& word) { words_.push_back(word); }
	Word* getMutableWord(int i) { reutrn &(words_[i]); }
	void setWords(const vector<Word>& words) {
		words_ = words; 
	}

	void addNewTable() {
		Table* new_table = new Table(0);
		tables_.push_back(new_table);
	}
	Table* getMutableTable(int i) { return tables_[i]; }

	int getId() const { return id_; }
	void setId(const int id) { id_ = id; }


private:
	// Id
	int id_;

	// Words.
	vector<Word> words_;

	// Table pointers.
	vector<Table*> tables_;  
};


}  // namespace hdp

#endif  // DOCUMENT_H_