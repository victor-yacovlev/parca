#ifndef PARCA_H
#define PARCA_H

#include "pareto.h"

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <boost/scoped_ptr.hpp>

#include <boost/python/dict.hpp>
#include <boost/python/list.hpp>
#include <boost/python/object.hpp>


class aligner
{
public:
    explicit aligner();
    explicit aligner(const aligner & other);
    explicit aligner(unsigned int limit, const std::string & process_id, unsigned int memory_usage);
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

private:
    boost::scoped_ptr<Aligner> m_aligner;


};

#endif
