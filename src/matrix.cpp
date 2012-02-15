#include "matrix.h"
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <boost/filesystem.hpp>
#include <set>

Matrix::Matrix()
{
    m_rows = 0;
    m_columns = 0;
}

Matrix::Matrix(int rows, int columns, int limit, const std::string &swapFileName, int storedRowsCount, bool dropRows)
{
    m_rows = rows;
    m_columns = columns;

    m_entrySize = sizeof(A);

    m_storedRowsCount = storedRowsCount;
    if (m_rows<m_storedRowsCount)
        m_storedRowsCount = m_rows;
    if (m_storedRowsCount<2)
        m_storedRowsCount = 2;
    m_memoryStorageBeginIndex = 0;
    m_dropRowsMode = dropRows;
    m_file = 0;
    m_filePath = boost::filesystem3::path(swapFileName);
    m_fileMode = FM_CLOSED;
    if (!m_dropRowsMode) {
        if (boost::filesystem3::exists(m_filePath))
            boost::filesystem3::remove(m_filePath);
        m_indeces = std::vector< std::vector<int> >(rows, std::vector<int>(columns, -1));
        m_sizes = std::vector< std::vector<int> >(rows, std::vector<int>(columns, 0));
    }
    m_storeRowsUsed = 2;
    m_limit = limit+1;

    alloc();
}

void Matrix::alloc()
{
    assert ( m_storedRowsCount>= 2);
    m_firstColumn = (A*)calloc(m_rows*m_limit,m_entrySize);
    m_storedRows = (A*)calloc(m_storedRowsCount*m_columns*m_limit, m_entrySize);
}

Matrix::~Matrix()
{
    if (m_file)
        fclose(m_file);
    if (boost::filesystem3::exists(m_filePath))
        boost::filesystem3::remove(m_filePath);
    
    free(m_storedRows);
    free(m_firstColumn);
}

int Matrix::size() const
{
    return m_rows;
}


std::deque<A> Matrix::get(int row, int col)
{


    assert ( row>=0 );
    assert ( row<m_rows );
    assert ( col>=0 );
    assert ( col<m_columns );

    int linearIndex = 0;

    if (col==0) {
        linearIndex = row*m_limit;
        std::deque<A> result;
        for (int i=0; i<m_limit; i++) {
            A a = m_firstColumn[linearIndex+i];
            if ( (a.l & STOP_FLAG) > 0 )
                break;
            result.push_back(a);
        }
        return result;
    }
    else if (row>=m_memoryStorageBeginIndex) {
        int rowIndex = row - m_memoryStorageBeginIndex;
        linearIndex = rowIndex*m_columns*m_limit + col*m_limit;
        std::deque<A> result;
        for (int i=0; i<m_limit-1; i++) {
            A a = m_storedRows[linearIndex+i];
            if ( (a.l & STOP_FLAG) > 0 )
                break;
            result.push_back(a);
        }
        return result;
    }
    else {
        return getFromFile(row, col);
    }
}

std::deque<A> Matrix::getLast()
{
    return get(m_rows-1, m_columns-1);
}

void Matrix::set(int row, int col, const std::deque<A> & value)
{
    A stop;
    stop.l = STOP_FLAG;

    assert ( row>=0 );
    assert ( row<m_rows );
    assert ( col>=0 );
    assert ( col<m_columns );

    int linearIndex = 0;


    if ( col==0 ) {
        linearIndex = row*m_limit;
        for (int i=0; i<value.size(); i++) {
            A a = value[i];
            m_firstColumn[linearIndex+i] = a;
        }
        if (value.size()<m_limit) {
            m_firstColumn[linearIndex+value.size()] = stop;
        }

    }
    else if (row>=m_memoryStorageBeginIndex && row<m_memoryStorageBeginIndex+m_storeRowsUsed) {
        int rowIndex = row - m_memoryStorageBeginIndex;
        linearIndex = rowIndex*m_columns*m_limit + col*m_limit;
        for (int i=0; i<value.size(); i++) {
            A a = value[i];            
            m_storedRows[linearIndex+i] = a;
        }
        if (value.size()<m_limit) {
            m_storedRows[linearIndex+value.size()] = stop;
        }
    }
    else {
        writeRowToStorage();
        int rowIndex = row - m_memoryStorageBeginIndex;
        assert ( rowIndex != -1  );
        linearIndex = rowIndex*m_columns*m_limit + col*m_limit;
        for (int i=0; i<value.size(); i++) {
            A a = value[i];
            m_storedRows[linearIndex+i] = a;
        }
        if (value.size()<m_limit) {
            m_storedRows[linearIndex+value.size()] = stop;
        }
    }
//    qDebug() << "Memory storage: ";
//    for (int i=0; i<m_storeRowsUsed; i++) {
//        qDebug() << "Row: " << i;
//        for (int j=0; j<m_columns; j++) {
//            qDebug() << "Column: " << j;
//            A a;
//            for (int k=0; k<m_limit; k++) {
//                a = m_storedRows[i][j*m_limit+k];
//                if ( (a.l & STOP_FLAG) > 0 )
//                    break;
//                qDebug() << a;
//            }
//        }
//    }
//    qDebug() << "End set " << row << ", " << col;
}

void Matrix::writeRowToStorage()
{
    if ( !m_dropRowsMode ) {
        if (m_storeRowsUsed==m_storedRowsCount) {
           writeStorageToFile();
        }
        else {
           m_storeRowsUsed++;
       }
    }
}




std::deque<A> Matrix::getFromFile(int row, int col)
{
    assert ( !m_dropRowsMode );
    // (пере)открываем файл на чтение
    if (m_fileMode==FM_WRITE) {
        fclose(m_file);
        m_file = 0;
        m_fileMode = FM_CLOSED;
    }
    if (m_fileMode!=FM_OPEN) {
#ifdef BOOST_WINDOWS_API
		m_file = _wfopen(m_filePath.c_str(), L"r");
#else
        m_file = fopen(m_filePath.c_str(), "r");
#endif
        m_fileMode = FM_OPEN;
    }
    
    // позиция в файле
    int index = m_indeces[row][col];
    int size = m_sizes[row][col];
    fseek(m_file, index, SEEK_SET);
    
    // чтение данных
    std::deque<A> result;
    for (int i=0; i<size; i++) {
        A a;
        fread(&a, sizeof(A), 1, m_file);
        result.push_back(a);
    }
    return result;
}

void Matrix::writeStorageToFile()
{
    // (пере)открываем файл на запись
    if (m_fileMode==FM_OPEN) {
        fclose(m_file);
        m_file = 0;
        m_fileMode = FM_CLOSED;
    }
    int index = 0;
    if (boost::filesystem3::exists(m_filePath)) {
        index = boost::filesystem3::file_size(m_filePath);
    }
    if (m_fileMode!=FM_WRITE) {
#ifdef BOOST_WINDOWS_API
		m_file = _wfopen(m_filePath.c_str(), L"w+");
#else
        m_file = fopen(m_filePath.c_str(), "w+");
#endif
		m_fileMode = FM_WRITE;
    }

    // скидываем в файл memory storage
    
    for (int row=0; row<m_storeRowsUsed; row++) {
        for (int col=0; col<m_columns; col++) {
            m_indeces[row+m_memoryStorageBeginIndex][col] = index;
            std::deque<A> set;
            for (int j=0; j<m_limit; j++) {
                int linearIndex = row*m_columns*m_limit + col*m_limit + j;
                A a = m_storedRows[linearIndex];
                if ( (a.l & STOP_FLAG) > 0 )
                    break;
                set.push_back(a);
            }
            m_sizes[row+m_memoryStorageBeginIndex][col] = set.size();
            for (int i=0; i<set.size(); i++) {
                A a = set[i];
                fwrite(&a, sizeof(A), 1, m_file);
                index += sizeof(A);
            }
        }
    }
    m_memoryStorageBeginIndex += m_storeRowsUsed;
    m_storeRowsUsed = 0;
    fflush(m_file);
}
