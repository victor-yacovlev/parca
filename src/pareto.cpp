#include "pareto.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <set>

Aligner::Aligner()
{
    MX = MY = M = 0;
    m_lastError = "";
    m_limit = 10;
    m_processId = "0";
    m_matrixInmemoryRows = 10;
    m_directStageDone = false;

}

void Aligner::init(uint32_t limit, const std::string &processId, uint32_t matrixInmemoryRows)
{
    reset(); // Clear previous state
    m_limit = limit;
    m_processId = processId;
    m_matrixInmemoryRows = matrixInmemoryRows;
    m_lastError = "";
//    std::cout << "Created aligner\n";
}


void Aligner::setScoreMatrix(const std::map<std::pair<wchar_t,wchar_t>, int32_t> &matrix)
{
    scores = matrix;
}

void Aligner::directStage(const std::wstring &SY, const std::wstring &SX)
{

    m_lastError = "";
    m_directStageDone = false;
    m_SY = SY;
    m_SX = SX;
    if (M!=NULL)
        delete M;
    if (MX!=NULL)
        delete MX;
    if (MY!=NULL)
        delete MY;
    M = MX = MY = NULL;
    if (SY.length()==0) {
        m_lastError = "First sequence is empty";
        return;
    }
    if (SX.length()==0) {
        m_lastError = "Second sequence is empty";
        return;
    }
    M  = new Matrix( SY.length()+1, SX.length()+1, m_limit, "aligner_"+m_processId+".main.swp", m_matrixInmemoryRows, false );
    MX = new Matrix( SY.length()+1, SX.length()+1, m_limit, "aligner_"+m_processId+".x.swp", m_matrixInmemoryRows, false );
    MY = new Matrix( SY.length()+1, SX.length()+1, m_limit, "aligner_"+m_processId+".y.swp", m_matrixInmemoryRows, false );

    /* First element initialization */

    std::deque<A> M_00, MX_00, MY_00;
    M_00.push_back(A(0,0,0,NIL));
    MX_00.push_back(A(0,0,1,DEL_H));
    MY_00.push_back(A(0,0,1,DEL_V));

    M ->set(0,0, M_00);
    MX->set(0,0, MX_00);
    MY->set(0,0, MY_00);

    /* First row initialization */

    for (int x=1; x<=SX.length(); x++) {
        std::deque<A> M_0x, MX_0x, MY_0x;
        M_0x.push_back(A(0,x,1,DEL_H));
        MX_0x.push_back(A(0,x,1,DEL_H));
        MY_0x.push_back(A(0,x,2,DEL_H));
        M ->set(0,x,M_0x);
        MX->set(0,x,MX_0x);
        MY->set(0,x,MY_0x);
    }

    /* First column initialization */

    for (int y=1; y<=SY.length(); y++) {
        std::deque<A> M_y0, MX_y0, MY_y0;
        M_y0.push_back(A(0,y,1,DEL_V));
        MX_y0.push_back(A(0,y,2,DEL_V));
        MY_y0.push_back(A(0,y,1,DEL_V));
        M ->set(y,0,M_y0);
        MX->set(y,0,MX_y0);
        MY->set(y,0,MY_y0);
    }

    /* Building matrices */

    std::deque<A> TX, TY, TD;
    std::deque<A> T;

    for (int y=1; y<=SY.length(); y++) {
        for (int x=1; x<=SX.length(); x++) {
            /* Main matrix [M] */
            TX = MX->get(y  ,x-1);
            TY = MY->get(y-1,x  );
            TD = M ->get(y-1,x-1);
            for (int i=0; i<TX.size(); i++) {
                TX[i].d ++;
                TX[i].l = DEL_H;
            }
            for (int i=0; i<TY.size(); i++) {
                TY[i].d ++;
                TY[i].l = DEL_V;
            }
            for (int i=0; i<TD.size(); i++) {
                wchar_t Sx = SX[x-1];
                wchar_t Sy = SY[y-1];
                int scoreToAdd = score(Sx, Sy);
                TD[i].m += int32_t(scoreToAdd);
                TD[i].l = NO_DEL;
            }
            T.clear();
            T.insert(T.end(), TX.begin(), TX.end());
            T.insert(T.end(), TY.begin(), TY.end());
            T.insert(T.end(), TD.begin(), TD.end());
            paretizeSet(T);
            M ->set(y,x,T);

            /* Secondary matrix [MX] */
            TX = MX->get(y  ,x-1);
            TD = M ->get(y  ,x  );
            for (int i=0; i<TX.size(); i++) {
                TX[i].d ++;
                TX[i].l = DEL_H;
            }
            for (int i=0; i<TD.size(); i++) {
                TD[i].g ++;
                TD[i].l = NO_DEL;
            }
            T.clear();
            T.insert(T.end(), TX.begin(), TX.end());
            T.insert(T.end(), TD.begin(), TD.end());
            paretizeSet(T);
            MX->set(y,x,T);

            /* Secondary matrix [MY] */
            TY = MY->get(y-1,x  );
            TD = M ->get(y  ,x  );
            for (int i=0; i<TY.size(); i++) {
                TY[i].d ++;
                TY[i].l = DEL_V;
            }
            for (int i=0; i<TD.size(); i++) {
                TD[i].g ++;
                TD[i].l = NO_DEL;
            }
            T.clear();
            T.insert(T.end(), TY.begin(), TY.end());
            T.insert(T.end(), TD.begin(), TD.end());
            paretizeSet(T);
            MY->set(y,x,T);

            if (m_lastError.length()>0) {
                return;
            }
        }
    }
    m_directStageDone = true;
}

void Aligner::paretizeSet(std::deque<A> &total)
{
    assert ( total.size()>0 );
    sort(total.begin(), total.end()); /* sort: maximum is first! */
    std::deque<A>::iterator prev;
    std::deque<A>::iterator cur;

    for (cur=total.begin(); cur!=total.end(); ) {
        if (m_limit>0 && cur->g>m_limit) {
            cur = total.erase(cur);
            if (cur!=total.begin())
                prev = cur - 1;
        }
        else {
            if (cur==total.begin()) {
                prev = cur;
                cur ++;
            }
            else {
                if (prev->g==cur->g && prev->m==cur->m) {
                    if (prev->d==cur->d) {
                        prev->l |= cur->l;
                        cur = total.erase(cur);
                        if (cur!=total.begin())
                            prev = cur - 1;
                    }
                    else if (prev->d < cur->d) {
                        cur = total.erase(cur);
                        if (cur!=total.begin())
                            prev = cur - 1;
                    }
                    else {
                        prev->d = cur->d;
                        prev->l = cur->l;
                        cur = total.erase(cur);
                        if (cur!=total.begin())
                            prev = cur - 1;
                    }
                }
                else if (prev->maj(*cur)) {
                    cur = total.erase(cur);
                    if (cur!=total.begin())
                        prev = cur - 1;
                }
                else {
                    prev = cur;
                    cur ++;
                }
            }
        }
    }
    assert ( total.size()>0 );
    std::set<uint16_t> gs;
    for (int i=0; i<total.size(); i++) {
        assert(gs.count(total[i].g)==0);
        gs.insert(total[i].g);
    }
}

int32_t Aligner::score(const wchar_t &ch1, const wchar_t &ch2)
{
    if (scores.size()==0) {
        if (m_lastError=="") {
            m_lastError = "Score matrix is empty";
        }
        return 0;
    }
    else {
        
        std::pair<wchar_t,wchar_t> pair(ch1,ch2);
        std::pair<wchar_t,wchar_t> pair2(ch2,ch1);
        if (scores.count(pair)) {
            return scores[pair];
        }
        else if (scores.count(pair2)) {
            return scores[pair2];
        }
        else {
            if (m_lastError.length()==0) {
                m_lastError = "No symbols pair in score matrix";
            }
            return 0;
        }
    }
}

uint32_t Aligner::resultCount()
{
    if (!m_directStageDone) {
        m_lastError = "Direct stage of alignment not done";
        return 0;
    }
    int y = m_SY.length();
    int x = m_SX.length();
    const std::deque<A> r = M->get(y,x);
    return r.size();
}


enum Path { Diagonal, Horizontal, Vertical, Undefined };

A Aligner::search_mgd(const std::deque<A> &set, int32_t m, uint16_t g, uint16_t d, bool &found)
{
    found = true;
    for ( int i=0; i<set.size(); i++ ) {
        if ( set[i].m==m && set[i].g==g && set[i].d==d )
            return set[i];
    }
    found = false;
    return A();
}


std::deque< std::pair<int16_t,int16_t> > Aligner::getAlignment(uint32_t mainElementNo)
{
    std::wstring SX = m_SX;
    std::wstring SY = m_SY;
    std::deque< std::pair<int16_t,int16_t> > result;
    if (!m_directStageDone) {
        m_lastError = "Direct stage of alignment not done";
        return result;
    }
    if ( (int)(mainElementNo) >= M->getLast().size() ) {
        m_lastError = "Main element index out of range";
        return result;
    }
    assert ( M != 0 ) ;
    assert ( (int)(mainElementNo) < M->getLast().size() );
    int row = M->rows()-1;
    int col = M->columns()-1;
    A e = M->getLast().at(mainElementNo);
    char matrix = 'd';
    std::pair<int16_t,int16_t> al;
    Path p;
    while ( row>0 && col>0 ) {
        assert ( e.l > 0 );
        p = Undefined;
        if ( (e.l&DEL_V)>0 )
            p = Vertical;
        else if ( (e.l&DEL_H)>0 )
            p = Horizontal;
        else if ( (e.l&NO_DEL)>0 )
            p = Diagonal;
        assert ( p!= Undefined );
        assert ( matrix=='d' || matrix=='v' || matrix=='h' );
        if ( matrix=='d' ) {
            if ( p==Vertical ) {
                assert( e.d > 0 );
                bool found = false;
                uint32_t _d = (uint32_t)(e.d - 1);
                int32_t _m = e.m;
                uint32_t _g = e.g;
                row--;
                std::deque<A> set = MY->get(row, col);
                matrix = 'v';
                e = search_mgd(set, _m, _g, _d, found);
                assert ( found );
                al.first = row;
                al.second = -1;
                result.push_front(al);
            }
            else if ( p==Horizontal ) {
                assert( e.d > 0 );
                bool found = false;
                uint32_t _d = (uint32_t)(e.d - 1);
                int32_t _m = e.m;
                uint32_t _g = e.g;
                col--;
                std::deque<A> set = MX->get(row, col);
                e = search_mgd(set, _m, _g, _d, found);
                matrix = 'h';
                assert ( found );
                al.first = -1;
                al.second = col;
                result.push_front(al);
            }
            else if ( p==Diagonal ) {
                wchar_t Sx = SX[col-1];
                wchar_t Sy = SY[row-1];
                bool found;
                int32_t scoreToAdd = score(Sx, Sy);
                int32_t _m = e.m - scoreToAdd;
                uint32_t _g = e.g;
                uint32_t _d = e.d;
                row--;
                col--;
                std::deque<A> set = M->get(row,col);
                e = search_mgd(set, _m, _g, _d, found);
                matrix = 'd';
                assert ( found );
                al.first = row;
                al.second = col;
                result.push_front(al);
            }
        }
        else if ( matrix=='h' ) {
            assert ( p==Horizontal || p==Diagonal );
            if ( p==Horizontal ) {
                assert( e.d > 0 );
                bool found = false;
                uint32_t _d = (uint32_t)(e.d - 1);
                int32_t _m = e.m;
                uint32_t _g = e.g;
                col--;
                std::deque<A> set = MX->get(row, col);
                e = search_mgd(set, _m, _g, _d, found);
                matrix = 'h';
                assert ( found );
                al.first = -1;
                al.second = col;
                result.push_front(al);
            }
            else if ( p==Diagonal ) {
                bool found;
                assert ( e.g > 0 );
                int32_t _m = e.m;
                uint32_t _g = (uint32_t)(e.g-1);
                uint32_t _d = e.d;
                std::deque<A> set = M->get(row,col);
                e = search_mgd(set, _m, _g, _d, found);
                matrix = 'd';
                assert ( found );
            }
        }
        else if ( matrix=='v' ) {
            assert ( p==Vertical || p==Diagonal );
            if ( p==Vertical ) {
                assert( e.d > 0 );
                bool found = false;
                uint32_t _d = (uint32_t)(e.d - 1);
                int32_t _m = e.m;
                uint32_t _g = e.g;
                row--;
                std::deque<A> set = MY->get(row, col);
                e = search_mgd(set, _m, _g, _d, found);
                matrix = 'v';
                assert ( found );
                al.first = row;
                al.second = -1;
                result.push_front(al);
            }
            else if ( p==Diagonal ) {
                bool found;
                assert ( e.g > 0 );
                int32_t _m = e.m;
                uint32_t _g = (uint32_t)(e.g-1);
                uint32_t _d = e.d;
                std::deque<A> set = M->get(row,col);
                e = search_mgd(set, _m, _g, _d, found);
                matrix = 'd';
                assert ( found );
            }
        }
    }
    while ( col>0 || row>0 ) {
        if ( col>0 ) {
            al.first = -1;
            al.second = col-1;
            result.push_front(al);
            col --;
        }
        else if ( row>0 ) {
            al.first = row-1;
            al.second = -1;
            result.push_front(al);
            row --;
        }
    }
//    qDebug() << result;
    return result;
}


alignment_info_t Aligner::getInfo(uint32_t no)
{
    alignment_info_t r;
    if (!m_directStageDone) {
        m_lastError = "Direct stage of alignment not done";
        return r;
    }
    std::deque<A> last = M->getLast();
    if ( (int)(no) >= last.size() ) {
        m_lastError = "Main element index out of range";
        return r;
    }
    r.m = last[no].m;
    r.d = last[no].d;
    r.g = last[no].g;
    return r;
}

void Aligner::reset()
{
    if (M!=NULL)
        delete M;
    if (MX!=NULL)
        delete MX;
    if (MY!=NULL)
        delete MY;
    if (boost::filesystem3::exists("aligner_"+m_processId+".main.swp")) {
        boost::filesystem3::remove("aligner_"+m_processId+".main.swp");
    }
    if (boost::filesystem3::exists("aligner_"+m_processId+".x.swp")) {
        boost::filesystem3::remove("aligner_"+m_processId+".x.swp");
    }
    if (boost::filesystem3::exists("aligner_"+m_processId+".y.swp")) {
        boost::filesystem3::remove("aligner_"+m_processId+".y.swp");
    }
    M = MX = MY = NULL;
    m_lastError = "";
    m_directStageDone = false;
}

Aligner::~Aligner()
{
    reset();
}

std::string Aligner::lastError() const
{
    return m_lastError;
}
