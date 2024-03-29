/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdexcept>
#include <sqlite3.h>

#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"

#include "ace/OS_NS_sys_stat.h"
#include <ace/OS_NS_sys_stat.h>
#include <ace/OS_NS_unistd.h>

#include "log.h"

#include "DataStoreReceiver.h"
#include "DataStoreDispatcher.h"
#include "DataStoreConfigManager.h"

using namespace ammo::gateway;

DataStoreReceiver::DataStoreReceiver (void)
  : db_ (0)
{
}

DataStoreReceiver::~DataStoreReceiver (void)
{
  LOG_DEBUG ("Closing Data Store Service database...");
	
  sqlite3_close (db_);
}

void
DataStoreReceiver::onConnect (GatewayConnector * /* sender */)
{
}

void
DataStoreReceiver::onDisconnect (GatewayConnector * /* sender */)
{
}

void
DataStoreReceiver::onPushDataReceived (GatewayConnector * /* sender */,
                                       PushData &pushData)
{
  dispatcher_.dispatchPushData (db_, pushData);
}

void
DataStoreReceiver::onPullRequestReceived (GatewayConnector *sender,
                                          PullRequest &pullReq)
{
  dispatcher_.dispatchPullRequest (db_, sender, pullReq);
}

void
DataStoreReceiver::db_filepath (const std::string &path)
{
  db_filepath_ = path;
}

bool
DataStoreReceiver::init (void)
{
  // The config manager's receiver and connectro references have been set,
  // so we can initialize the dispatcher's member with a call using the
  // default null arguments.
  dispatcher_.set_cfg_mgr (DataStoreConfigManager::getInstance ());

  try
    {
      if (!check_path ())
        {
          // check_path() will also output error info.
          LOG_ERROR ("DataStoreReceiver::init() failed");
          return false;
        }
        
      std::string fullpath (db_filepath_);
      fullpath += "/DataStore_db.sql3";
      
    //  LOG_DEBUG ("full path = " << fullpath.c_str ());
      
      int status = sqlite3_open (fullpath.c_str (), &db_);
      
      if (status != 0)
        {
          LOG_ERROR ("Data Store Service - " << sqlite3_errmsg (db_));
          return false;
        }
      
      const char *data_tbl_str =
	      "CREATE TABLE IF NOT EXISTS data_table ("
	      "uri TEXT,"
	      "mime_type TEXT,"
	      "origin_user TEXT,"
	      "tv_sec INTEGER NOT NULL,"
	      "tv_usec INTEGER,"
	      "data BLOB)";
	
      char *db_err = 0;
	
      sqlite3_exec (db_, data_tbl_str, 0, 0, &db_err);
	
      if (db_err != 0)
	      {
	        LOG_ERROR ("Data Store Service data table - " << db_err);
			    return false;
	      }
	  }
	catch (const std::exception &ex)
    {
      LOG_ERROR ("DataStoreReceiver::init: caught std::exception - "
                 << ex.what ());
      return false;
    }
  catch (...)
    {
      LOG_ERROR ("DataStoreReceiver::init: caught unknown C++ exception");
      return false;
    }
	  
  return true;
}

bool
DataStoreReceiver::check_path (void)
{
#ifdef WIN32
  const char delimiter = '\\';
  const char *delim_str = "\\";
#else
  const char delimiter = '/';
  const char *delim_str = "/";
#endif
  
  std::string::size_type lastPos =
    db_filepath_.find_first_not_of (delimiter, 0);
  std::string::size_type pos =
    db_filepath_.find_first_of (delimiter, lastPos);
  
  std::string seg = db_filepath_.substr (lastPos, pos - lastPos);
  bool top_level = true;
  
  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      //LOG_DEBUG ("segment: " << seg);
      int result = 0;
      
      switch (db_filepath_[0])
        {
          case delimiter:
            result =
              ACE_OS::chdir (top_level
                             ? (std::string (delim_str) + seg).c_str ()
                             : seg.c_str ());
            break;
          case '$':
            result =
              ACE_OS::chdir (top_level
                             ? ACE_OS::getenv (seg.c_str () + 1)
                             : seg.c_str ());
            break;
          default:
            result = ACE_OS::chdir (seg.c_str ());
            break;
        }

      if (result == -1)
        {
          LOG_ERROR ("check_path(); error changing current directory to "
                     << seg);
          return false;
        }
  
      lastPos = db_filepath_.find_first_not_of (delimiter, pos);
      pos = db_filepath_.find_first_of (delimiter, lastPos);
      
      // This would get caught in the next iteration but
      // we need to check and possibly exit here.
      if (std::string::npos == lastPos)
        {
          return true;
        }
        
      seg = db_filepath_.substr (lastPos, pos - lastPos);
      
      top_level = false;
      result = ACE_OS::mkdir (seg.c_str (), S_IRWXU | S_IRWXG | S_IRWXO);
      
      // EEXIST is ok - directory already existed, ignore the return value.
      if (result != 0 && errno != EEXIST)
        {
          LOG_ERROR ("check_path(): error creating db filepath directory "
                     << seg);
          return false;
        }
    }
    
  // Make sure it ends with a slash so we can just append the db filename.  
  if (db_filepath_[db_filepath_.size () - 1] != delimiter)
    {
      db_filepath_ += delimiter;
    }
    
  return true;
}


