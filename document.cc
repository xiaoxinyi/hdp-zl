#include "document.h"

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


// =======================================================================
// TableUtils
// =======================================================================


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


}  // namespace hdp