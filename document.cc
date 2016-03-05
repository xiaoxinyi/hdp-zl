#include <math.h>
#include <assert.h>

#include "document.h"
#include "utils.h"

namespace hdp {

// =======================================================================
// Word
// =======================================================================

Word::Word(int id) 
		: id_(id) {

}

Word::~Word() {}


// =======================================================================
// WordUtils
// =======================================================================
void WordUtils::UpdateTableFromWord(Word* word,
																	  int update) {
	Table* table = word->getMutableTable();
	table->incWordCount(update);
	if (update == 1) {
		table->addWordId(word->getId());
	}

	if (update == -1) {
		table->removeWordId(word->getId());
	}

	TableUtils::UpdateTopic(table, word->getId(), update);
}

// =======================================================================
// Table
// =======================================================================

Table::Table()
		: word_count_(0) {
	topic_ = NULL;
}

Table::Table(int word_count)
		: word_count_(word_count) {
	topic_ = NULL;
}

Table::removeWordId(int word_id) {
	removeFromVec(word_ids_, word_id);
}

// =======================================================================
// TableUtils
// =======================================================================

void TableUtils::UpdateTopic(Table* table,
														int word_id,
														int update) {
	Topic* topic = table->getMutableTopic();
	topic->updateWordCounts(word_id, update);
}

void TableUtils::UpdateTopicFromTable(Table* table
																			int update) {
	int words = table->getWordCount();

	for (int i = 0; i < words; i++) {
		int word_id = table->getWordId(i);
		UpdateTopic(table, word_id, update);
	}

	table->GetaMutableTopic()->incTableCount(update);
}


// =======================================================================
// Document
// =======================================================================

Document::Document(int id) 
		: id_(id) {
}

Document::~Document() {
	int size = getTables();
	for (int i = 0; i < size; i++) {
		if (tables_ != NULL) {
			delete tables_[i];			
		}
	}
}

// =======================================================================
// DocumentUtils
// =======================================================================


void DocumentUtils::SampleTables(Document* document,
																 bool remove,
																 double alpha,
											    			 double gamma) {
	// TODO
}

void DocumentUtils::SampleTableForWord(Document* document,
																			 Word* word,
																 			 bool remove,
																 			 double alpha,
											    			 			 double gamma) {
	if (remove) WordUtils::UpdateTableFromWord(word, -1);

	int tables = document->getTables();
	vector<double> log_pr(tables + 1, log(0));

	AllTopics all_topics = AllTopics.GetInstance();
	int topics = all_topics.getTopics();
	assert(topics > 0);

	int corpus_word_no = all_topics.getMutableTopic(0).getCorpusWordNo();
	int word_id = word->getId();

	// Log_word_pr for word in each topic.
	vector<double> f(topics+1, log(0));
	double f_new = log(gamma / corpus_word_no);

	int total_tables = 0;

	for (int k = 0; k < topics; i++) {
		Topic* topic = all_topics.getMutableTopic(k);
		f[k] = topic->getLogWordPr(word_id);
		int table_count = topic->getTableCount();
		total_tables += table_count;
		f_new = Utils::LogSum(f_new, 
													f[k] + log(table_count)));
	}
	f_new = f_new - log(total_tables + gamma);
	f[topics] = f_new;
}
}  // namespace hdp