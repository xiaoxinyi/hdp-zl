#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <vector>
#include <unordered_map>

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


// This class provides the functionality for updating 
// the table statistics correspond to the word.
class WordUtils {
public:

// updating the table statistics correspond to the word.
static void UpdateTableFromWord(Word* word, int update);
};


// This class has number of words in a table.
// Topic assignment of the table.
class Table {
public:
	Table();
	Table(word_count);

	int getWordCount() const { return word_count_; }
	void setWordCount(const int& word_count) { word_count_ = word_count; }
	void incWordCount(int val) { word_count_ += val; }

	Topic* getMuableTopic() { return topic; }
	void setTopic(Topic* topic) { topic_ = topic; }

	int getCountById(int word_id);
	void updateMapWordCount(int word_id, int update);

	unordered_map<int, int>& getMapWordCount() const { return map_word_count_; }
private:
	// Number of words.
	int word_count_;

	// Word ids assigned to the table.
	unordered_map<int, int> map_word_count_;

	// A pointer to topoic the table is assigned to.
	Topic* topic_;
};

// The class provides functionality for updating topic's
// statistics correspond to table.
class TableUtils {
public:
// Get word_ids and counts in the table.
static void GetWordsAndCounts(Table* table,
															vector<int>& word_ids,
															vector<int>& counts);

// Updating topic's statistics correspond to table.
// and the word id.
static void UpdateTopicFromTable(Table* table,
														int word_id,
														int update);

// Remove or add table to a topic.
// word_ids - all the word_ids in table.
// counts - correspond word count in table.
// remove - if true remove table from topic, 
// otherwise add table to topic.
static void UpdateTopicFromTable(Table* table,
																 vector<int>& word_ids,
																 vector<int>& counts,
													 			 int update);

// Sample a topic for a table.
static void SampleTopicForTable(Table* table);
};

// The Document contains an id, a number of words.
// A number of table pointers.
class Document {
public:
	Document(int id);
	~Document();

	int getWords() { return words_.size(); }
	int getTables() { return tables_.size(); }

	void addWord(int id) { words_.push_back(Word(id)); }
	void addWord(const Word& word) { words_.push_back(word); }
	Word* getMutableWord(int i) { reutrn &(words_[i]); }
	void setWords(const vector<Word>& words) {
		words_ = words; 
	}

	void addNewTable() {
		Table* new_table = new Table(0);
		tables_.push_back(new_table);
	}
	void removeTable(int pos);

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

// The class provide functionality for 
// sampling table for each word.
class DocumentUtils {
public:
	// Sample table for each word.
	// Dirichlet process parameter,
	// alpha - second level,
	// gamma - first level.
	static void SampleTables(Document* document,
													 int permute_words,
											     bool remove,
											     double alpha,
											     double gamma);

	// Sample table for one word.
	// Dirichlet process parameter,
	// alpha - second level,
	// gamma - first level.
	static void SampleTableForWord(Document* document,
																 Word* word,
																 bool remove,
																 double alpha,
											    			 double gamma);

	// Sample topic for each table given a document.
	static void SampleTopics(Document* document);
	
	// Compress the unused tables.
	static void CompactTables(Document* document);

	// Compute alpha score - document partition score.
	static double AlphaScore(Document* document, 
													 double alpha);

	// Permute words in the document.
	static void PermuteWords(Document* document);
};


}  // namespace hdp

#endif  // DOCUMENT_H_