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

install_github('repo','ref')

* repo is a GitHub repository path, for example `paradigm4/chunk_unique`
* ref is a GitHub branch, for example `master`

## Requirements
This plugin requires a few things:

* SciDB development headers need to be installed. For example, for SciDB
version 14.8 run the following for Ubuntu or RHEL/CentOS, repsectively:
Ubuntu:
```
sudo apt-get install scidb-14.8-dev scidb-14.8-libboost1.54-all-dev
```
CentOS or RHEL:
```
sudo yum install scidb-14.8-dev  scidb-14.8-libboost-devel
```

* The user that runs the `scidb` process must have read/write access to
the `lib/scidb/plugins` directory on every computer that SciDB is installed
on in the cluster. For example, if the SciDB user name is `scidb` with
group name `scidb`, and the
computer nodes in the cluster are `10.0.0.1, 10.0.0.2, 10.0.0.3`,  then
run the following as the root user:
```
ssh 10.0.0.1 "chown -R scidb:scidb /opt/scidb/14.8/lib/scidb/plugins
ssh 10.0.0.2 "chown -R scidb:scidb /opt/scidb/14.8/lib/scidb/plugins
ssh 10.0.0.3 "chown -R scidb:scidb /opt/scidb/14.8/lib/scidb/plugins
```
* The wget and tar programs must be installed on the coordinator node.
* Installation must be initiated from the coordinator node.
* The plugin repository must build with the simple command `make`.

## Example

Install the example 'chunk_unique' (`cu`) operator:

```
load_library('dev_tools')
install_github('paradigm4/chunk_unique','master')
load_library('cu')
```


## Installing the plug in

You'll need SciDB installed, along with the SciDB development header packages.
The names vary depending on your operating system type, but they are the
package that have "-dev" in the name. You *don't* need the SciDB source code to
compile and install this.

Run `make` and copy  the `libknn.so` plugin to the `lib/scidb/plugins`
directory on each of your SciDB cluster nodes. Here is an example:

```
cd dev_tools
make
cp *.so /opt/scidb/14.8/lib/scidb/plugins

iquery -aq "load_library('dev_tools')"
```
Remember to copy the plugin to *all* your SciDB cluster nodes.
