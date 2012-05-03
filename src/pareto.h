#ifndef PARETO_H
#define PARETO_H

#include "matrix.h"
#include "a.h"
#include <vector>
#include <deque>
#include <map>
#include <utility>
#include <wchar.h>
#include <string>
#include <boost/cstdint.hpp>

typedef std::pair<wchar_t,wchar_t> matrix_element_key_t;
typedef std::map<matrix_element_key_t,int32_t> matrix_t;
typedef std::vector<short> alignment_pair_t;
typedef std::vector<alignment_pair_t> alignment_data_t;

struct alignment_info_t {
    int m;
    unsigned short g;
    unsigned short d;
};

struct alignment_t {
    int m;
    unsigned short g;
    unsigned short d;
    alignment_data_t data;
};

class Aligner
{
public:
    Aligner();
    void init(uint32_t limit, const std::string &processId, uint32_t matrixInmemoryRows);
    void setScoreMatrix(const matrix_t &matrix);
    void setTempDir(const std::string &path);
    void directStage(const std::wstring &SY, const std::wstring &SX);
    uint32_t resultCount();
    std::deque< std::pair<int16_t,int16_t> > getAlignment(uint32_t no);
    alignment_info_t getInfo(uint32_t no);
    std::string lastError() const;
    void reset();
    ~Aligner();
private:
    void paretizeSet(std::deque<A> &total);
    int32_t score(const wchar_t &ch1, const wchar_t &ch2);
    A search_mgd(const std::deque<A> &set, int32_t m, uint16_t g, uint16_t d, bool &found);
    Matrix *MX;
    Matrix *MY;
    Matrix *M;
    std::wstring m_SX;
    std::wstring m_SY;
    std::string m_lastError;
    std::string m_tempDir;
    uint32_t m_limit;
    std::string m_processId;
    uint32_t m_matrixInmemoryRows;
    std::map<std::pair<wchar_t,wchar_t>, int32_t> scores;
    bool m_directStageDone;
};

#endif // PARETO_H
