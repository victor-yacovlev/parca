# norootforbuild


%define python_name_prefix python


Name:           parca
Version:        1.0.0
Release:        0
License:        MIT
Vendor:         IMPB RAS
Packager:       Victor Yacovlev <victor@lpm.org.ru>

Source:         %name-%version.tar.gz

Requires:       %python_name_prefix-%name = %version
Requires:       %_bindir/python2.7

BuildRequires:  %_bindir/g++
BuildRequires:  cmake >= 2.8.3
BuildRequires:  pkgconfig(python-2.7)
BuildRequires:  boost-devel >= 1.46

Summary:        PAReto CAndidate sequence aligner

%description
PARCA is a global alignment tool for aminoacid sequences.
Instead of classical Smith-Waterman algorhitm (implemented
in EMBOSS and, partially, FASTA), PARCA results to a set
of alignments candidates. 

Usually you do not need PARCA to align high-gomologious
(at least 30\% initial identity) sequences, but PARCA
shows higher accuracy than Smith-Waterman algorhitm in
case of low-gomologious sequences.

%prep
%setup -q

%build
mkdir build
pushd build
cmake ../ -DCMAKE_INSTALL_PREFIX=%buildroot/%_prefix
make
popd

%install
pushd build
make install 
popd
mkdir -p %buildroot/%_bindir/
cp parca %buildroot/%_bindir/

%clean
rm -rf %buildroot

%files
%defattr(-,root,root)
%dir %_bindir
%_bindir/parca

%package -n %python_name_prefix-%name
Summary:        Python module for PARCA
Requires:       %_libdir/libpython2.7.so
Requires:       %_libdir/libboost_filesystem-mt.so
Requires:       %_libdir/libboost_python-mt.so
Requires:       %_libdir/libboost_system-mt.so

%description -n %python_name_prefix-%name
Python module for PARCA scripting

%files -n %python_name_prefix-%name
%defattr(-,root,root)
%dir %_libdir/python2.7/site-packages/parca
%_libdir/python2.7/site-packages/parca/*
