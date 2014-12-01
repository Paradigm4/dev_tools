/*
 *    _____      _ ____  ____
 *   / ___/_____(_) __ \/ __ )
 *   \__ \/ ___/ / / / / __  |
 *  ___/ / /__/ / /_/ / /_/ / 
 * /____/\___/_/_____/_____/  
 *
 *
 * BEGIN_COPYRIGHT
 *
 * This file is part of SciDB.
 * Copyright (C) 2008-2014 SciDB, Inc.
 *
 * SciDB is free software: you can redistribute it and/or modify
 * it under the terms of the AFFERO GNU General Public License as published by
 * the Free Software Foundation.
 *
 * SciDB is distributed "AS-IS" AND WITHOUT ANY WARRANTY OF ANY KIND,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY,
 * NON-INFRINGEMENT, OR FITNESS FOR A PARTICULAR PURPOSE. See
 * the AFFERO GNU General Public License for the complete license terms.
 *
 * You should have received a copy of the AFFERO GNU General Public License
 * along with SciDB.  If not, see <http://www.gnu.org/licenses/agpl-3.0.html>
 *
 * END_COPYRIGHT
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "query/Operator.h"
#include "query/Network.h"

#define CMDBUFSZ 16384

namespace scidb
{

class Physicalinstall_github: public PhysicalOperator
{
public:
    Physicalinstall_github(string const& logicalName,
                           string const& physicalName,
                           Parameters const& parameters,
                           ArrayDesc const& schema):
        PhysicalOperator(logicalName, physicalName, parameters, schema)
    {
    }

    shared_ptr<Array> execute(vector<shared_ptr<Array> >& inputArrays,
            shared_ptr<Query> query)
    {
        char cmd[CMDBUFSZ];
        char dir[4096];
        int k;
        char *d;
        Instances instances;
        SystemCatalog::getInstance()->getInstances(instances);
        Instances::const_iterator iter = instances.begin();
        const char *scidbstor = iter->getPath().c_str();

// Only in the inscrutable SciDB code would the following intuitive check not
// work. Argghhhhh.
//        if (query->getCoordinatorID() == query->getInstanceID())
        if (query->getCoordinatorID() == COORDINATOR_INSTANCE)
        {
            const char *s = "";
            InstanceID id;
            std::set<InstanceID> first_instances;
            while(iter != instances.end())
            {
              if(strcmp(s,iter->getHost().c_str())!=0)
              {
                s = iter->getHost().c_str();
                id = iter->getInstanceId();
                first_instances.insert(id);
              }
              ++iter;
            }

            std::string const repo = ((boost::shared_ptr<OperatorParamPhysicalExpression>&)_parameters[0])->getExpression()->evaluate().getString();
            std::string const branch = ((boost::shared_ptr<OperatorParamPhysicalExpression>&)_parameters[1])->getExpression()->evaluate().getString();

            snprintf(dir,4096,"/tmp/install_github_XXXXXX");
            d = mkdtemp(dir);
            if(!d) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to create temp directory";
            snprintf(cmd,CMDBUFSZ,"cd %s && wget https://github.com/%s/archive/%s.tar.gz",dir,repo.c_str(),branch.c_str());
            k = ::system((const char *)cmd);
            if(k!=0) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to retrieve repository";

// Get our base SciDB path (XXX is there an easier way?), and build the plugin
            memset(cmd,0,CMDBUFSZ);
            snprintf(cmd,CMDBUFSZ,"x=%s;x=`readlink $x/SciDB-*`;x=`dirname $x`;x=`dirname $x`;cd %s; tar -zxf *;cd %s/*-%s;SCIDB=$x make && tar -zcf ../plugin.tar.gz *.so",scidbstor,dir,dir,branch.c_str());
            k = ::system((const char *)cmd);
            if(k!=0) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to build plugin";

// Read the compiled plugin into a buffer for distribution
            memset(cmd,0,CMDBUFSZ);
            snprintf(cmd, CMDBUFSZ, "%s/plugin.tar.gz", dir);
            struct stat sb;
            if (stat(cmd, &sb) == -1) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to stat plugin";
            shared_ptr<SharedBuffer> tarball(new MemoryBuffer(NULL, sb.st_size * sizeof(char)));
            shared_ptr<SharedBuffer> nothing(new MemoryBuffer(NULL, 0));
            FILE *f = fopen(cmd, "r");
            if (!f) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to open plugin";
            size_t sz = fread((char *)tarball->getData(), sizeof(char), sb.st_size, f);
            fclose(f);
            if(sz != sb.st_size*sizeof(char)) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to read plugin";

// Install the plugin locally
            memset(cmd,0,CMDBUFSZ);
            snprintf(cmd,CMDBUFSZ,"x=%s;x=`readlink $x/SciDB-*`;x=`dirname $x`;x=`dirname $x`;x=$x/lib/scidb/plugins/;cd %s;tar -C $x -zxf plugin.tar.gz;cd ..;rm -rf %s",scidbstor,dir,dir);
            k = ::system((const char *)cmd);
            if(k!=0) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to install plugin";

// Copy the plugin to other instances and install it
            for(InstanceID i = 0; i < query->getInstancesCount(); i++)
            {  
                if(i == query->getInstanceID())
                {   continue; }
                else if(first_instances.find(i) != first_instances.end())
                {
                  // copy file
                  BufSend(i, tarball, query);
                } else
                {
                  // don't copy
                  BufSend(i, nothing, query);
                }
            }

            shared_ptr<Array> outputArray(new MemArray(_schema, query));
            shared_ptr<ArrayIterator> outputArrayIter = outputArray->getIterator(0);
            Coordinates position(1, 0);
            shared_ptr<ChunkIterator> outputChunkIter = outputArrayIter->newChunk(position).getIterator(query, 0);
            outputChunkIter->setPosition(position);
            Value value;
            value.setBool(true);
            outputChunkIter->writeItem(value);
            outputChunkIter->flush();
            return outputArray;
        }

// Non-coordinator instances...
        shared_ptr<SharedBuffer> buf = BufReceive(query->getCoordinatorID(), query);
        if(buf)
        {
            snprintf(dir,4096,"/tmp/install_github_XXXXXX");
            d = mkdtemp(dir);
            if(!d) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to create temp directory";
            memset(cmd,0,CMDBUFSZ);
            snprintf(cmd, CMDBUFSZ, "%s/plugin.tar.gz", dir);
            FILE *f = fopen(cmd, "w+");
            if (!f) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to open plugin";
            size_t sz = fwrite((char *)buf->getData(), sizeof(char), buf->getSize(), f);
            fclose(f);
            buf->free();
            if(sz != buf->getSize()*sizeof(char)) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to write plugin";
// Install the plugin locally
            memset(cmd,0,CMDBUFSZ);
            snprintf(cmd,CMDBUFSZ,"x=%s;x=`readlink $x/SciDB-*`;x=`dirname $x`;x=`dirname $x`;x=$x/lib/scidb/plugins/;cd %s;tar -C $x -zxf plugin.tar.gz;cd ..; rm -rf %s",scidbstor,dir,dir);
            k = ::system((const char *)cmd);
            if(k!=0) throw SYSTEM_EXCEPTION(SCIDB_SE_OPERATOR, SCIDB_LE_ILLEGAL_OPERATION)
                        << "failed to install plugin";

        }
        return boost::shared_ptr<Array>(new MemArray(_schema,query));
    }
};

REGISTER_PHYSICAL_OPERATOR_FACTORY(Physicalinstall_github, "install_github", "Physicalinstall_github");

} //namespace
