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
	table->updateMapWordCount(word->getId(), update);
	

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



int Table::getCountById(int word_id) {
	auto it = map_word_count_.find(word_id);
	if (it == word_id.end()) {
		return 0;
	} else {
		return map_word_count_[word_id];
	}
}

void updateMapWordCount(int word_id, int update) {
	auto it = map_word_count_.find(word_id);
	if (it == word_id.end()) {
		map_word_count_[word_id] = update;
	} else {
		map_word_count_[word_id] += update;
	}
	assert(map_word_count_[word_id] >= 0);
	if (map_word_count_[word_id] == 0) {
		map_word_count_.erase(word_id);
	}
	
	return;
}

// =======================================================================
// TableUtils
// =======================================================================

void TableUtils::GetWordsAndCounts(Table* table,
																	 vector<int>& word_ids,
																	 vector<int>& counts) {
	unordered_map<int, int>& m = table->getMapWordCount();
	int size = m.size();

	word_ids.reserve(size);
	counts.reserve(size);

	for (auto p : m) {
		word_ids.push(p.first);
		counts.push(p.second);
	}
}

void TableUtils::UpdateTopic(Table* table,
														int word_id,
														int update) {
	Topic* topic = table->getMutableTopic();
	topic->updateWordCounts(word_id, update);
}

void TableUtils::UpdateTopicFromTable(Table* table,
																			vector<int>& word_ids,
																			vector<int>& counts,
																			int update) {
	
	int size = word_ids.size();

	for (int i = 0; i < size; i++) {
		int word_id = word_ids[i];
		UpdateTopic(table, word_id, update * counts[word_id]);
	}

	table->GetaMutableTopic()->incTableCount(update);
}

void TableUtils::SampleTopicForTable(Table* table, 
																		 double gamma) {
	AllTopics& all_topics = AllTopics::GetInstance();
	int topics = all_topics.getTopics();

	vector<int> word_ids;
	vector<int> counts;
	TableUtils::GetWordsAndCounts(table, word_ids, counts);
	
	assert(topics >= 1);
	int corpus_word_no = all_topics.getMutableTopic[0]->getCorpusWordNo();
	all_topics.addNewTopic(corpus_word_no);

	vector<double> log_pr(topics + 1, 0.0);
	for (int i = 0; i < topics; i++) {
		Topic* topic = all_topics.getMutableTopic(i);
		if (table->getMutableTopic() != topic) {
			log_pr[i] = TopicTableUtils::LogGammaRatio(table, topic, 
																								 word_ids, counts) +
									log(topic->getTableCount());
		} else {
			log_pr[i] = TopicTableUtils::LogGammaRatio(table, topic
																								 word_ids, counts) +
									log(topic->getTableCount() - 1);
		}
		Topic* new_topic = all_topics.getMutableTopic(topics);
		log[topics] = log(gamma) + TopicTableUtils::LogGammaRatio(table, new_topic
																															word_ids, counts);
								
	}

	int sample_topic = Utils::SampleFromLogPr(log_pr);
	Topic* old_topic = table->getMutableTopic();
	Topic* new_topic = all_topics.getMutableTopic(sample_topic);

	if (old_topic != new_topic) {
		table->setTopic(new_topic);
		TableUtils::UpdateTopicFromTable(table, word_ids, counts, 1);
	}

	if (sample_topic == topics) {
		all_topics.removeLastTopic();
	}
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

void Table::removeTable(int pos) {
	Table* table = tables_[pos];
	assert(table->getWordCount() == 0);
	assert(table->getMapWordCount().size() == 0);
	Topic* topic = table->getMutableTopic();
	topic->incTableCount(-1);
	topic = NULL;
	auto it = tables_.begin() + pos;
	delete *it;
	*it = NULL;
	tables_.erase(it)
}

// =======================================================================
// DocumentUtils
// =======================================================================

double DocumentUtils::AlphaScore(Document* document, 
													 			 double alpha) {
	int tables = document->getTables();
	int words = document->getWords();
	double score = 0.0;
	score += tables * log(alpha) +
					 gsl_sf_lngamma(alpha) - 
					 gsl_sf_lngamma(words + alpha);
	for (int i = 0; i < tables; i++) {
		Table* table = document->getMutableTable(i);
		score += gsl_sf_lngamma(table->getWordCount());
	}
	return score;
}

void DocumentUtils::CompactTables(Document* document) {
	int tables = document->getTables();
	for (int i = tables - 1; i >=0 ; --i) {
		if (document->getMutableTable(i)->getWords() == 0) {
			document->removeTable(i);	
		}	
	}
}

void DocumentUtils::SampleTables(Document* document,
																 int permute_words,
																 bool remove,
																 double alpha,
											    			 double gamma) {
	if (permute_words == 1) {
		PermuteWords(document);
	}

	int words = document->getWords();
	for (int i = 0; i < words; i++) {
		Word* word = document->getMutableWord(i);
		SampleTableForWord(document, word, remove, alpha, gamma);
	}
}

void DocumentUtils::SampleTableForWord(Document* document,
																			 Word* word,
																 			 bool remove,
																 			 double alpha,
											    			 			 double gamma) {
	if (remove) WordUtils::UpdateTableFromWord(word, -1);

	int tables = document->getTables();
	vector<double> log_pr(tables + 1, log(0));

	AllTopics& all_topics = AllTopics.GetInstance();
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

void DocumentUtils::SampleTopics(Document* document) {
	int tables = document->getTables();
	for (int i = 0; i < tables; i++) {
		Table* table = document->getMutableTable(i);
		TableUtils::SampleTopicForTable(table);
	}
}

void DocumentUtils::PermuteWords(Document* document) {
  int size = document->getWords();
  vector<Word> permuted_words;

  // Permute the values in perm.
  // These values correspond to the indices of the words in the
  // word vector of the document.
  gsl_permutation* perm = gsl_permutation_calloc(size);
  Utils::Shuffle(perm, size);
  int perm_size = perm->size;
  assert(size == perm_size);

  for (int i = 0; i < perm_size; i++) {
    permuted_words.push_back(*document->getMutableWord(perm->data[i]));
  }

  document->setWords(permuted_words);

  gsl_permutation_free(perm);
}

}  // namespace hdp
