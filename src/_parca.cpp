
#include <Python.h>
#include "_parca.h"
#include "pareto.h"
#include "matrix.h"
#include <iostream>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <wchar.h>

aligner::aligner()
{
    m_aligner = new Aligner();
}

aligner::aligner(unsigned int limit, const std::string & processId, unsigned int matrixInmemoryRows)
{
    m_aligner = new Aligner();
    m_aligner->init((uint32_t)limit,processId,(uint32_t)matrixInmemoryRows);
}

void aligner::init(unsigned int limit, const std::string & processId, unsigned int matrixInmemoryRows)
{
    uint32_t l = (uint32_t)limit;
    std::string pid = processId;
    uint32_t m = (uint32_t)matrixInmemoryRows;
    m_aligner->init(l,pid,m);
}


void aligner::set_score_matrix(const matrix_t & matrix)
{
    m_aligner->setScoreMatrix(matrix);
}


void aligner::set_temporary_directory(const std::string & path)
{
    m_aligner->setTempDir(path);
}


void aligner::py_set_score_matrix(const boost::python::dict &d)
{
    matrix_t matrix;
    using namespace boost::python;
    const list items = d.items();
    for (int i=0; i<len(items); i++) {
        const tuple item = extract<tuple>(items[i]);
        const tuple key = extract<tuple>(item[0]);
        const int val = extract<int>(item[1]);
        const std::wstring s1 = extract<std::wstring>(key[0]);
        const std::wstring s2 = extract<std::wstring>(key[1]);
        assert(s1.length()>=1);
        assert(s2.length()>=1);
        matrix_element_key_t mkey(s1[0], s2[0]);
        matrix[mkey] = (int32_t)val;
    }
    set_score_matrix(matrix);
}

void aligner::process_direct_stage(const std::wstring & SY, const std::wstring & SX)
{

    m_aligner->directStage(SX,SY);
}

int aligner::result_count()
{
    return (int)(m_aligner->resultCount());
}

alignment_t aligner::get_alignment(unsigned int no)
{
    std::deque< std::pair<int16_t,int16_t> > r = m_aligner->getAlignment(no);
    alignment_info_t info = get_alignment_info(no);
    alignment_t result;
    result.m = info.m; result.g = info.g; result.d = info.d;
    for (int i=0; i<r.size(); i++) {
        alignment_pair_t p(2);
        p[0] = r[i].second;
        p[1] = r[i].first;
        result.data.push_back(p);
    }
    return result;
}

alignment_info_t aligner::get_alignment_info(unsigned int no)
{
    return m_aligner->getInfo(no);
}

std::string aligner::get_last_error() const
{
    return m_aligner->lastError();
}

void aligner::reset()
{
    m_aligner->reset();
}

aligner::~aligner()
{
    delete m_aligner;
}

void aligner::selftest_matrix()
{
    const uint16_t SZ = 1000;
    const uint16_t MZ = 160;
    std::cerr << "Begin self test\n";
    Matrix * m = new Matrix(SZ,SZ,40,"self-test",MZ,false);
    // Write
    for (int y=0; y<SZ; y++) {
        for (int x=0; x<SZ; x++) {
            std::deque<A> elems;
            for (int i=0; i<40; i++) {
                A a(x*y,2*i,i,0);
                elems.push_back(a);
            }
            m->set(y,x,elems);
        }
    }
    // Read'n'check
    for (int y=0; y<SZ; y++) {
        for (int x=0; x<SZ; x++) {
            std::deque<A> elems = m->get(y,x);
            assert(elems.size()==40);
            for (int i=0; i<40; i++) {
                A a = elems[i];
                assert(a.m == x*y);
                assert(a.d == 2*i);
                assert(a.g == i);
            }
        }
    }
    delete m;
    std::cerr << "Self test OK\n";
}


BOOST_PYTHON_MODULE(_parca)
{
    using namespace boost::python;
    using namespace std;


    class_<alignment_pair_t>("alignment_pair")
            .def(vector_indexing_suite<alignment_pair_t>())
            ;

    class_<alignment_data_t>("alignment_data")
            .def(vector_indexing_suite<alignment_data_t>())
            ;

    class_<alignment_info_t>("alignment_info")
            .def_readwrite("m", &alignment_info_t::m)
            .def_readwrite("d", &alignment_info_t::d)
            .def_readwrite("g", &alignment_info_t::g)
            ;

    class_<alignment_t>("alignment")
            .def_readwrite("m", &alignment_t::m)
            .def_readwrite("d", &alignment_t::d)
            .def_readwrite("g", &alignment_t::g)
            .def_readwrite("data", &alignment_t::data)
            ;

    class_<aligner>("aligner")
        .def(init<unsigned int, string, unsigned int>())
        .def("init",&aligner::init)
        .def("set_score_matrix", &aligner::py_set_score_matrix)
        .def("process_direct_stage", &aligner::process_direct_stage)
        .def("result_count", &aligner::result_count)
        .def("get_alignment", &aligner::get_alignment)
        .def("get_alignment_info", &aligner::get_alignment_info)
        .def("get_last_error", &aligner::get_last_error)
        .def("reset", &aligner::reset)
        .def("selftest_matrix_1", &aligner::selftest_matrix)
        .def("set_temporary_directory", &aligner::set_temporary_directory)
    ;
}

