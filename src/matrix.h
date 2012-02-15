#ifndef MATRIX_H
#define MATRIX_H

#include "a.h"
#include <string>
#include <deque>
#include <boost/filesystem.hpp>

class Matrix
{
  public:
    Matrix();
    Matrix(int rows, int columns, int limit, const std::string &swapFilename, int storedRowsCount, bool dropRows);
    std::deque<A> get(int row, int col);
    std::deque<A> getLast();
    void set(int row, int col, const std::deque<A> &value);
    int size() const;
    inline int columns() const { return m_columns; }
    inline int rows() const { return m_rows; }
    ~Matrix();
  private:
    void alloc();
    size_t m_entrySize;
    std::deque<A> getFromFile(int row, int col);
    void writeRowToStorage();
    void writeStorageToFile();
    enum FileMode { FM_CLOSED, FM_OPEN, FM_WRITE };
    FILE * m_file;
    boost::filesystem3::path m_filePath;
    FileMode m_fileMode;
    A* m_storedRows;
    A* m_firstColumn;
    int m_rows;
    int m_columns;
    int m_storedRowsCount;
    int m_memoryStorageBeginIndex;
    int m_limit;
    int m_storeRowsUsed;
    int m_storageOffset;
    std::vector< std::vector<int> > m_indeces;
    std::vector< std::vector<int> > m_sizes;
    bool m_dropRowsMode;
};

#endif // MATRIX_H
