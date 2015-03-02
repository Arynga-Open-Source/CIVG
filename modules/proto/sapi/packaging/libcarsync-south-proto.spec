Name:           libcarsync-south-proto
Version:        0.2.0-2~2.gbp785070
Release:        1
License:        Proprietary
Summary:        Library with CarSync South API
Url:            http://arynga.com
Group:          Environment/Base
Source0:        %{name}-%{version}.tar.gz
Source1001:     packaging/%{name}.manifest
BuildRequires:  automake
BuildRequires:  libtool
BuildRequires:  protobuf-compiler
BuildRequires:  protobuf-devel
BuildRequires:  CarSync-HU-Common

%description
Arynga Inc.
Library with CarSync South API

%prep
%setup -q
./autogen.sh

%build
cp %{SOURCE1001} .
CFLAGS="%{optflags} -D_GNU_SOURCE"

%configure

make %{?_smp_mflags} %{?jobs:-j%jobs} BINDIR=%{_bindir}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}%{_mandir}/man1
cp -a %{SOURCE1001} %{buildroot}%{_mandir}/man1
rm -fr %{buildroot}%{_prefix}/usr/lib/debug/.build-id/

%files
%manifest %{name}.manifest
#%{_bindir}/*
#%{_datadir}/*
%{_libdir}/libcarsync*
#%{_libdir}/pkgconfig/*
#%{_libdir}/ts/*
#%{_sysconfdir}/*
%{_includedir}/*
%doc %{_mandir}/*/*

%changelog
* Thu Sep 26 2013 Maciek Borzecki <maciej.borzecki@open-rnd.pl> - 0.2.0-2~2.gbp785070-1
- Include headers directory

* Tue Jul 23 2013 Bartlomiej Jozwiak <bartlomiej.jozwiak@open-rnd.pl> - 0.0.1-2
- Initial packaging

