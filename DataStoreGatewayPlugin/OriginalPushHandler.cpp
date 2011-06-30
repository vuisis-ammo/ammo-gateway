#include <sqlite3.h>

#include "ace/OS_NS_sys_time.h"

#include "GatewayConnector.h"
#include "log.h"

#include "OriginalPushHandler.h"
#include "DataStoreUtils.h"

OriginalPushHandler::OriginalPushHandler (sqlite3 *db,
                                          const ammo::gateway::PushData &pd)
  : PushHandler (db, pd)
{
}

bool
OriginalPushHandler::handlePush (void)
{
  ACE_Time_Value tv (ACE_OS::gettimeofday ());
	
  int status =
	  sqlite3_prepare (db_,
			               "insert into data_table values (?,?,?,?,?,?)",
			               -1,
			               &stmt_,
			               0);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Data push - "
                 << "prep of sqlite statement failed: "
		             << sqlite3_errmsg (db_));
		
      return false;
    }
    
  bool good_binds =
    DataStoreUtils::bind_text (db_, stmt_, 1, pd_.uri, true)
    && DataStoreUtils::bind_text (db_, stmt_, 2, pd_.mimeType, true)
    && DataStoreUtils::bind_text (db_, stmt_, 3, pd_.originUsername, true)
    && DataStoreUtils::bind_int (db_, stmt_, 4, tv.sec ())
    && DataStoreUtils::bind_int (db_, stmt_, 5, tv.usec ())
    && DataStoreUtils::bind_blob (db_, stmt_, 6, pd_.data.data (), pd_.data.length (), true);

  if (good_binds)
    {	
      status = sqlite3_step (stmt_);
	
      if (status != SQLITE_DONE)
        {
          LOG_ERROR ("Data push - "
                     << "insert operation failed: "
                     << sqlite3_errmsg (db_));
		
          return false;
        }
    }

  return good_binds;
}
