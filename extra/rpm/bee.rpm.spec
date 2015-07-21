####################################################
################# MACROS AND DEFAULTS ##############
####################################################

%{?scl:%global _scl_prefix /opt/bee}
%{?scl:%scl_package bee}

%define _source_filedigest_algorithm 0
%define _binary_filedigest_algorithm 0

%global debug_package %{nil}
%global _enable_debug_package %{nil}
%global __debug_install_post %{nil}
%global __debug_package %{nil}

%if (0%{?fedora} >= 15 || 0%{?rhel} >= 7) && %{undefined _with_systemd}
%global _with_systemd 1
%endif

%bcond_with    systemd

BuildRequires: readline-devel

%if 0%{?rhel} < 7 && 0%{?rhel} > 0
BuildRequires: cmake28
BuildRequires: devtoolset-2-toolchain
BuildRequires: devtoolset-2-binutils-devel
%else
BuildRequires: cmake >= 2.8
BuildRequires: gcc >= 4.5
BuildRequires: binutils-devel
%endif

%if 0%{?fedora} > 0
BuildRequires: perl-podlators
%endif

Requires(pre): /usr/sbin/useradd
Requires(pre): /usr/sbin/groupadd
%if %{with systemd}
Requires(post): systemd
Requires(preun): systemd
Requires(postun): systemd
BuildRequires: systemd
%else
Requires(post): chkconfig
Requires(post): initscripts
Requires(preun): chkconfig
Requires(preun): initscripts
%endif

# Strange bug.
# Fix according to http://www.jethrocarr.com/2012/05/23/bad-packaging-habits/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Name: %{?scl_prefix}bee
Version: 
Release: 
Group: Applications/Databases
Summary: Bee - a NoSQL database in a Lua script
Vendor: bee.org
License: BSD
Requires: readline
Provides: %{?scl_prefix}bee-debuginfo
Provides: %{?scl_prefix}bee-debug
Requires: %{?scl_prefix}bee-common
URL: http://bee.org
Source0: 
%description
Bee is a high performance in-memory NoSQL database. It supports
replication, online backup, stored procedures in Lua.

This package provides the server daemon and administration
scripts.

# Bee dev spec
%package dev
Summary: Bee C connector and header files
Vendor: bee.org
Group: Applications/Databases
Requires: %{?scl_prefix}bee = -
%description dev
Bee is a high performance in-memory NoSQL database.
It supports replication, online backup, stored procedures in Lua.

This package provides Bee client libraries.

%package common
Summary: Bee common files
Vendor: bee.org
Group: Applications/Databases
Provides: %{?scl_prefix}bee-common
%if 0%{?rhel} != 5
BuildArch: noarch
%endif
Requires: %{?scl_prefix}bee
%description common
Bee is a high performance in-memory NoSQL database.
It supports replication, online backup, stored procedures in Lua.

This package provides common files

##################################################################

%prep
%setup -n 

%build
# https://fedoraproject.org/wiki/Packaging:RPMMacros

%{lua:
    local function is_rhel_old()
        local version = tonumber(rpm.expand('0%{?rhel}'))
        return (version < 7 and version > 0)
    end
    function wrap_with_toolset(cmd)
        local cmd = rpm.expand(cmd)
        local devtoolset = 'scl enable devtoolset-2 %q\n'
        if is_rhel_old() then
            return string.format(devtoolset, cmd)
        end
        return cmd
    end
    local function cmake_key_value(key, value)
        return " -D"..key.."="..value
    end
    local function dev_with (obj, flag)
        local status = "OFF"
        if tonumber(rpm.expand("%{with "..obj.."}")) ~= 0 then
            status = "ON"
        end
        return cmake_key_value(flag, status)
    end
    local function dev_with_kv (obj, key, value)
        if tonumber(rpm.expand("%{with "..obj.."}")) ~= 0 then
            return cmake_key_value(key, value)
        end
        return ""
    end
    local cmd = 'cmake'
    if is_rhel_old() then
        cmd = 'cmake28'
    end
    cmd = cmd .. ' . '
        .. cmake_key_value('CMAKE_BUILD_TYPE', 'RelWithDebugInfo')
        .. cmake_key_value('ENABLE_BACKTRACE', 'ON')
        .. cmake_key_value('CMAKE_INSTALL_PREFIX', '%{_prefix}')
        .. cmake_key_value('CMAKE_INSTALL_BINDIR', '%{_bindir}')
        .. cmake_key_value('CMAKE_INSTALL_LIBDIR', '%{_libdir}')
        .. cmake_key_value('CMAKE_INSTALL_LIBEXECDIR', '%{_libexecdir}')
        .. cmake_key_value('CMAKE_INSTALL_SBINDIR', '%{_sbindir}')
        .. cmake_key_value('CMAKE_INSTALL_SHAREDSTATEDIR', '%{_sharedstatedir}')
        .. cmake_key_value('CMAKE_INSTALL_DATADIR', '%{_datadir}')
        .. cmake_key_value('CMAKE_INSTALL_INCLUDEDIR', '%{_includedir}')
        .. cmake_key_value('CMAKE_INSTALL_INFODIR', '%{_infodir}')
        .. cmake_key_value('CMAKE_INSTALL_MANDIR', '%{_mandir}')
        .. cmake_key_value('CMAKE_INSTALL_LOCALSTATEDIR', '%{_localstatedir}')
        .. ' %{!?scl:-DCMAKE_INSTALL_SYSCONFDIR=%{_sysconfdir}}'
        .. ' %{!?scl:-DENABLE_RPM=ON}'
        .. ' %{?scl:-DENABLE_RPM_SCL=ON}'
        .. dev_with('systemd', 'WITH_SYSTEMD')
        .. dev_with_kv('systemd', 'SYSTEMD_SERVICES_INSTALL_DIR', '%{_unitdir}')

    print(wrap_with_toolset(cmd))}
%{lua:print(wrap_with_toolset('make %{?_smp_mflags}\n'))}

%install
make VERBOSE=1 DESTDIR=%{buildroot} install

%pre
/usr/sbin/groupadd -r bee > /dev/null 2>&1 || :
%if 0%{?rhel} < 6
/usr/sbin/useradd -M -g bee -r -d /var/lib/bee -s /sbin/nologin\
    -c "Bee Server" bee > /dev/null 2>&1 || :
%else
/usr/sbin/useradd -M -N -g bee -r -d /var/lib/bee -s /sbin/nologin\
    -c "Bee Server" bee > /dev/null 2>&1 || :
%endif

%post common
mkdir -m 0755 -p %{_var}/run/bee/
chown bee:bee %{_var}/run/bee/
mkdir -m 0755 -p %{_var}/log/bee/
chown bee:bee %{_var}/log/bee/
mkdir -m 0755 -p %{_var}/lib/bee/
chown bee:bee %{_var}/lib/bee/
mkdir -m 0755 -p %{_sysconfdir}/bee/instances.enabled/
mkdir -m 0755 -p %{_sysconfdir}/bee/instances.available/

%if %{with systemd}
%systemd_post bee.service
%else
chkconfig --add bee
/sbin/service bee start
%endif

%preun common
%if %{with systemd}
%systemd_preun bee.service
%else
/sbin/service bee stop
chkconfig --del bee
%endif

%postun common
%if %{with systemd}
%systemd_postun_with_restart bee.service
%endif

%files
%defattr(-,root,root,-)

"%{_bindir}/bee"

%dir "%{_datadir}/doc/bee"
"%{_datadir}/doc/bee/README.md"
"%{_datadir}/doc/bee/LICENSE"

"%{_mandir}/man1/bee.1.gz"

%files dev
%defattr(-,root,root,-)
%dir "%{_includedir}/bee"
"%{_includedir}/bee/lauxlib.h"
"%{_includedir}/bee/luaconf.h"
"%{_includedir}/bee/lua.h"
"%{_includedir}/bee/lua.hpp"
"%{_includedir}/bee/luajit.h"
"%{_includedir}/bee/lualib.h"
"%{_includedir}/bee/bee.h"

%files common
%defattr(-,root,root,-)
"%{_bindir}/beectl"
"%{_mandir}/man1/beectl.1.gz"
"%{_sysconfdir}/sysconfig/bee"
%if %{with systemd}
%dir "%{_libdir}/bee/"
"%{_unitdir}/bee.service"
"%{_libdir}/bee/bee.init"
%else
"%{_sysconfdir}/init.d/bee"
%endif

%changelog
* Tue Apr 28 2015 roman@bee.org <roman@bee.org> 1.0-3
- Remove sql-module, pg-module, mysql-module
* Fri Jun 06 2014 Eugine Blikh <bigbes@bee.org> 1.0-2
- Add SCL support
- Add --with support
- Add dependencies
* Mon May 20 2013 Dmitry Simonenko <support@bee.org> 1.0-1
- Initial version of the RPM spec
