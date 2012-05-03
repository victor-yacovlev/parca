#ifndef PARCA_H
#define PARCA_H

#include <string>
#include <map>
#include <utility>
#include <vector>
#include <list>
#include <utility>
#include <map>
#include <string>
#include <boost/python/dict.hpp>
#include <boost/python/list.hpp>
#include <boost/python/object.hpp>

#include "pareto.h"

class Aligner;


class aligner
{
public:
    aligner();
    aligner(unsigned int limit, const std::string & process_id, unsigned int memory_usage);
    void init(unsigned int limit, const std::string & process_id, unsigned int memory_usage);
    void set_score_matrix(const matrix_t & matrix);
    void set_temporary_directory(const std::string &path);
    void py_set_score_matrix(const boost::python::dict& d);
    void process_direct_stage(const std::wstring & SY, const std::wstring & SX);
    int result_count();
    alignment_t get_alignment(unsigned int no);
    alignment_info_t get_alignment_info(unsigned int no);
    std::string get_last_error() const;
    void reset();
    void selftest_matrix();
    ~aligner();
private:
    Aligner *m_aligner;


};

#endif
