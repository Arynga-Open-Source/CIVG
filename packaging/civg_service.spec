Summary:    CarSync In Vehicle Gateway
Name:       civg
Version:    2.0.0
Release:    20.201150220
Group:      System/Utilities
License:    MPL-2.0
Source:     %{name}-%{version}.tar.gz

BuildRequires:  make
BuildRequires:  cmake
BuildRequires:  glib2-devel
BuildRequires:  openssl-devel
BuildRequires:  sqlite3-devel
BuildRequires:  libprotobuf-devel
BuildRequires:  dbus-glib-devel
BuildRequires:  libsoup-devel
BuildRequires:  sqlite3-devel
BuildRequires:  rpm
BuildRequires:  git
BuildRequires:  libwebsockets
BuildRequires:  zlib-devel
Requires:       tar
Requires:       dumm

%description
CARSYNC Node running on Tizen.

%prep
%setup -c carsync-$RPM_PACKAGE_VERSION

%build
%cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=""
make

%install
%make_install
mkdir -p %{buildroot}%{_libdir}/systemd/system
cp -a packaging/civg_service.service $RPM_BUILD_ROOT%{_libdir}/systemd/system/

%post
/bin/civg_config /var/lib/civg/config/config_example.json
cd %{_libdir}/systemd/system/multi-user.target.wants
ln -fs ../civg_service.service .
systemctl daemon-reload
systemctl enable civg_service


%clean
rm -rf $RPM_BUILD_ROOT

%changelog
* Mon Oct 27 2014 Piotr Lauk <piotr.lauk@arynga.com> - 2.0.0-19.20141027git19e2a4c
- Update to rev 19e2a4c to include more robust demoreset script

* Mon Oct 27 2014 Piotr Lauk <piotr.lauk@arynga.com> - 2.0.0-18.20141027git4603a2c
- Update to rev 4603a2c to include demoreset change that cleans configs and stops all services

* Mon Oct 27 2014 Piotr Lauk <piotr.lauk@arynga.com> - 2.0.0-17.20141027git64318ce
- Update to rev 64318ce to include fixes for notifying about available update 

* Mon Oct 27 2014 Piotr Lauk <piotr.lauk@arynga.com> - 2.0.0-16.20141027git0c8085d
- Update to rev 0c8085d to include fixes regarding adapter error notification 

* Mon Oct 27 2014 Maciej Jablonski <maciej.jablonski@arynga.com> - 2.0.0-15.20141027git4a88027
- Updated WS adapter and version 

* Mon Oct 27 2014 Piotr Lauk <piotr.lauk@arynga.com> - 2.0.0-14.20141027git4e98afd
- Update to rev 4e98afd to include rvi_consumer and civg_service fixes 

* Mon Oct 27 2014 Maciek Borzecki <maciek.borzecki@open-rnd.pl> - 2.0.0-13.20141027gite3c4215
- Update to rev e3c4215 to include ws_adapter fixes

* Mon Oct 27 2014 Maciek Borzecki <maciek.borzecki@gmail.com> - 2.0.0-12.20141027gite709068e
- Update to rev 5231ba to include core fixes. civg_service would
  segfault when asking for pending updates

* Mon Oct 27 2014 Maciek Borzecki <maciej.borzecki@open-rnd.pl> - 2.0.0-11.20141027gite5231ba
- Update to rev e5231ba to include config module race condition fixes

* Sat Oct 25 2014 Maciek Borzecki <maciek.borzecki@gmail.com> - 2.0.0-10.20141023git67e12e4
- Update to rev 91367a6 to include fixes for RVI messages (timeout and
  JSON response).

* Sat Oct 25 2014 Maciek Borzecki <maciek.borzecki@gmail.com> - 2.0.0-9.20141023git91367a6
- Update to rev 9ec2509 to include RVI 0.2.1 support

* Sat Oct 25 2014 Maciek Borzecki <maciej.borzecki@open-rnd.pl> - 2.0.0-8.20141023git9ec2509
- Update to rev 53aae8d to include CheckUpdates JSON-RPC method

* Sat Oct 25 2014 Maciek Borzecki <maciek.borzecki@open-rnd.pl> - 2.0.0-7.20141023git53aae8d
- Update rev to dd0c1f6, includes fixes for WebSocket adapter, reset
  script and initial package version

* Thu Oct 23 2014 Maciek Borzecki <maciek.borzecki@open-rnd.pl> - 2.0.0-6.20141023gitdd0c1f6
- Update to rev 9aacbf1, includes fixes to image type names

* Thu Oct 23 2014 Maciek Borzecki <maciek.borzecki@open-rnd.pl> - 2.0.0-5.20141023git4f44221
- Update to rev 4f44221, includes default config for demo

* Thu Oct 23 2014 Maciek Borzecki <maciej.borzecki@open-rnd.pl> - 2.0.0-4.20141023git0cee27d
- Update to rev 0cee27d to include RVI request handling fixes

* Thu Oct 23 2014 Maciek Borzecki <maciej.borzecki@open-rnd.pl> - 2.0.0-3.20141023gitbf756b0
- Update to rev bf756b0 to include logging changes, debug output
  enabled by default

* Thu Oct 23 2014 Maciek Borzecki <maciej.borzecki@open-rnd.pl> - 2.0.0-2.20141023git1ffbf7a
- Update to rev 1ffbf7a, to include RVI node registration changes

%files 
%manifest packaging/civg_service.manifest 
%defattr(-,root,root)
/bin/civg_service
/bin/civg_config
%dir /var/log/civg
/var/lib/civg/config/config_example.json
%{_libdir}/systemd/system/*
