// cloner.h - copy a database (export/import basically)

/**
 *    Copyright (C) 2011 10gen Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#pragma once

#include "mongo/base/disallow_copying.h"
#include "mongo/client/dbclientinterface.h"
#include "mongo/s/catalog/catalog_manager.h"

namespace mongo {

struct CloneOptions;
class DBClientBase;
class NamespaceString;
class OperationContext;


class Cloner {
    MONGO_DISALLOW_COPYING(Cloner);

public:
    Cloner();

    void setConnection(DBClientBase* c) {
        _conn.reset(c);
    }

    /**
     * Copies an entire database from the specified host.
     */
    Status copyDb(OperationContext* txn,
                  const std::string& toDBName,
                  const std::string& masterHost,
                  const CloneOptions& opts,
                  std::set<std::string>* clonedColls);

    bool copyCollection(OperationContext* txn,
                        const std::string& ns,
                        const BSONObj& query,
                        std::string& errmsg,
                        bool copyIndexes);

private:
    void copy(OperationContext* txn,
              const std::string& toDBName,
              const NamespaceString& from_ns,
              const BSONObj& from_opts,
              const NamespaceString& to_ns,
              bool masterSameProcess,
              const CloneOptions& opts,
              Query q);

    void copyIndexes(OperationContext* txn,
                     const std::string& toDBName,
                     const NamespaceString& from_ns,
                     const BSONObj& from_opts,
                     const NamespaceString& to_ns,
                     bool masterSameProcess,
                     bool slaveOk);

    struct Fun;
    std::unique_ptr<DBClientBase> _conn;
};

/**
 *  slaveOk     - if true it is ok if the source of the data is !ismaster.
 *  useReplAuth - use the credentials we normally use as a replication slave for the cloning
 *  snapshot    - use snapshot mode for copying collections.  note this should not be used
 *                when it isn't required, as it will be slower.  for example,
 *                repairDatabase need not use it.
 *  checkForCatalogChange - Internal option set for clone commands initiated by a mongos that are
 *                holding a distributed lock (such as movePrimary).  Indicates that we need to
 *                be periodically checking to see if the catalog manager has swapped and fail
 *                if it has so that we don't block the mongos that initiated the command.
 */
struct CloneOptions {
    std::string fromDB;
    std::set<std::string> collsToIgnore;

    bool slaveOk = false;
    bool useReplAuth = false;
    bool snapshot = true;

    bool syncData = true;
    bool syncIndexes = true;
    bool checkForCatalogChange = false;
};

}  // namespace mongo
