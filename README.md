# dev_tools

Tools that support installation of SciDB plugins from GitHub repositories.

## Description

The `dev_tools` plugin library contains functions and operators for
working with SciDB code in GitHub repositories.

The `install_github` operator can install plugins directly from GitHub
repositories, taking care of downloading, compiling, and distributing the
plugin to all the SciDB nodes.

Other development tools and functions are expected to be added to this plugin
library over time.

## Synopsis
```
install_github('repo' [, 'branch'] [, 'options'])
```
where, square brackets indicate option arguments and:
* repo is a GitHub repository path, for example 'paradigm4/chunk_unique'
* branch is an a GitHub branch, defaults to 'master'
* options is an optional string of environment variable definitions preceeding
  the invocation of make, for example 'SCIDB_3RDPARTY=/somewhere'

The environment variable SCIDB is automatically set to the path containing
the running version of scidb. You can set the SCIDB environment variable
in the options string to override the default choice.

## Installation

### Pre-requisites

* The user that runs the `scidb` process must have read/write access to
the `lib/scidb/plugins` directory on every computer that SciDB is installed on
in the cluster (that is, on every *node*). For example, if the SciDB user name
is `scidb` with group name `scidb`, and the computer nodes in the cluster are
`10.0.0.1, 10.0.0.2, 10.0.0.3`,  then run the following as the root user:
```
ssh 10.0.0.1 "chown -R scidb:scidb /opt/scidb/15.12/lib/scidb/plugins
ssh 10.0.0.2 "chown -R scidb:scidb /opt/scidb/15.12/lib/scidb/plugins
ssh 10.0.0.3 "chown -R scidb:scidb /opt/scidb/15.12/lib/scidb/plugins
```
* The wget and tar programs must be installed on the SciDB node on which SciDB instance ID 0 is running, and the tar program must be installed on all SciDB nodes.
* Installation must be initiated from instance ID 0.
* The plugin repository must build with the simple command `make` and the
optional environment variable definitions passed through the options argument.
* Note that plugins that rely on system libraries require that those libraries are available on all cluster nodes.
* SciDB should be installed at /opt/scidb. Otherwise you will need to supply the SCIDB and SCIDB_THIRDPARTY_PREFIX variables when building and installing plugins.

### Required packages: SciDB 15.12
If you are building SciDB from source, you will not have access to the `paradigm4-15.12-dev` package but the headers it provides should already be installed at /opt/scidb/15.12/include. You should be able to proceed without that package.

Assuming you are starting with a fairly bare system, you will need these packages:
```
#On Ubuntu 14.04:
#Omit paradigm4-15.7-dev if you've built and installed from source
sudo apt-get install paradigm4-15.12-dev make git scidb-15.12-libboost1.54-dev g++-4.9 gcc-4.9 libpqxx-dev liblog4cxx10-dev

#On CentOS 6:
#Install the compiler from a third-party repo:
sudo yum install scl-utils
wget https://www.softwarecollections.org/en/scls/rhscl/devtoolset-3/epel-6-x86_64/download/rhscl-devtoolset-3-epel-6-x86_64.noarch.rpm
sudo rpm -i rhscl-devtoolset-3-epel-6-x86_64.noarch.rpm 
#Omit paradigm4-15.7-dev if you've built and installed from source
sudo yum install paradigm4-15.12-dev git devtoolset-3-gcc-c++.x86_64 scidb-15.12-libboost-devel libpqxx-devel log4cxx-devel
```

### Required packages: SciDB 15.7
Release 15.7 uses the newer C++ compiler that must be installed separately. If you are building SciDB from source, you will not have access to the `paradigm4-15.7-dev` package but the headers it provides should already be installed at /opt/scidb/15.7/include. You should be able to proceed without that package.

Assuming you are starting with a fairly bare system, you will need these packages:
```
#On Ubuntu 14.04:
#Omit paradigm4-15.7-dev if you've built and installed from source
sudo apt-get install paradigm4-15.7-dev make git scidb-15.7-libboost1.54-dev g++-4.9 gcc-4.9 libpqxx-dev liblog4cxx10-dev

#On CentOS 6:
#Install the compiler from a third-party repo:
sudo yum install scl-utils
wget https://www.softwarecollections.org/en/scls/rhscl/devtoolset-3/epel-6-x86_64/download/rhscl-devtoolset-3-epel-6-x86_64.noarch.rpm
sudo rpm -i rhscl-devtoolset-3-epel-6-x86_64.noarch.rpm 
#Omit paradigm4-15.7-dev if you've built and installed from source
sudo yum install paradigm4-15.7-dev git devtoolset-3-gcc-c++.x86_64 scidb-15.7-libboost-devel libpqxx-devel log4cxx-devel
```

### Required packages: SciDB 14.12 or earlier
A little simpler
```
# On Ubuntu systems, run:
sudo apt-get install scidb-14.12-dev scidb-14.12-libboost1.54-all-dev libpqxx3-dev liblog4cxx10-dev

# On CentOS or RHEL systems, run:
sudo yum install scidb-14.12-dev  scidb-14.12-libboost-devel libpqxx-devel log4cxx-devel
```
Note that some plugins might require installation of additional operating system development
header packages. If a plugin fails to compile, examine its error output and install additional
packages as required.

### Build and install the plugin

Run `make` and copy  the `libdev_tools.so` plugin to the `lib/scidb/plugins`
directory on each of your SciDB cluster nodes. Here is an example:

```
cd dev_tools
make
cp *.so /opt/scidb/15.12/lib/scidb/plugins

iquery -aq "load_library('dev_tools')"
```
Remember to copy the plugin to *all* your SciDB cluster nodes.

## Example

Install the example `grouped_aggregate` operator:

```
load_library('dev_tools')
install_github('paradigm4/grouped_aggregate')
load_library('grouped_aggregate')
```

If your SciDB installation is not under /opt/scidb/, or if you are using an older version, you may need additional options:
```
load_library('dev_tools')
install_github('paradigm4/grouped_aggregate', 'v15.7', 'SCIDB=/home/apoliakov/workspace/15.7/stage/install SCIDB_THIRDPARTY_PREFIX=/opt/scidb/15.7')
load_library('grouped_aggregate')
```
Obviously that requires the plugin makefile to respect these variables.

## What happens under the hood

Install_github uses the wget command to download the compressed, tarball
repository from GitHub on SciDB node 0 only.
For example, `install_github('paradigm4/knn','master')`
will result in a wget of `https://github.com/paradigm4/knn-master.tar.gz`.
It then decompresses the tarball and tries to build the plugin by issuing
the `make` command in the top-level directory of the decompressed
repository.

If that succeeds, install_github then bundles any resulting `*.so` files
into a single compressed tarball and copies them using the internal
SciDB network stack to one instance on each computer in the SciDB cluster.
The receiving instances decompress the built library into their
respective `lib/scidb/plugins` directories.

## Errors

If things go wrong, errors are reported to the SciDB log. Errors from
invoked command-line programs like wget and tar are reported in the
`scidb-stderr.log` file on each instance.
