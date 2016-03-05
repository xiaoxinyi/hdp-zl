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

	// Log_word_pr + log(table number) in each topic.
	// log_pr_topic is log probabilities for sampling topics.
	vector<double> log_pr_topic(topics + 1 , 0.0);

	// f is Log_word_pr for word in each topic.
	vector<double> f(topics+1, log(0));
	double f_new = log(gamma / corpus_word_no);

	log_pr_topic[topics] = f_new;

	int total_tables = 0;
	unordered_map<Topic*, int> topic_to_k;

	for (int k = 0; k < topics; i++) {
		Topic* topic = all_topics.getMutableTopic(k);
		topic_to_k[topic*] = k;
		f[k] = topic->getLogWordPr(word_id);
		int table_count = topic->getTableCount();
		total_tables += table_count;
		log_pr_topic[k] = f[k] + log(table_count);
		f_new = Utils::LogSum(f_new, 
													f[k] + log(table_count)));
	}
	f_new = f_new - log(total_tables + gamma);
	f[topics] = f_new;

	for (int t = 0; t < tables; t++) {
		Table* table = document->getMutableTable(t);
		int table_word_count = table.getWordCount();
		double log_word_count = log(table_word_count);
	
		Topic* topic = table->getMutableTopic();
		int k = topic_to_k[topic];

		log_pr[t] = log_word_count + f[k];
	}

	log_pr[tables] = log(alpha) + f[topics];

	int sample_table = Utils::SampleFromLogPr(log_pr);
	
	// Sampled table is a new table or old.
	if (sample_table == tables) {
		// Table is new created.
		document->addNewTable();
		Table* new_table = document->getMutableTable(sample_table);
		word->setTable(new_table);
		
		int sample_topic = Utils::SampleFromLogPr(log_pr_topic);
		if (sample_topic == topics) {
			// Sampled topic is new.
			all_topics.addNewTopic(corpus_word_no);	
		}

		// Set topic for new table.
		Topic* new_topic = all_topics.getMutableTopic(sample_topic);
		new_table->setTopic(new_topic);
	} else {
		// Table is not new created. 
		// Set table for word.
		Table* new_table = document->getMutableTable(sample_table);
		word->setTable(new_table);
	}

	// Update statistics of table and topic from word
	WordUtils::UpdateTableFromWord(word, 1);
}


}  // namespace hdp