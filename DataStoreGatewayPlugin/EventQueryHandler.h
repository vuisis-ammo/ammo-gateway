#ifndef EVENT_QUERY_HANDLER_H
#define EVENT_QUERY_HANDLER_H

#include "OriginalQueryHandler.h"
#include "ProjectionFilter.h"

class EventQueryHandler : public OriginalQueryHandler,
                          public ProjectionFilter
{
public:
  EventQueryHandler (sqlite3 *db,
                     ammo::gateway::GatewayConnector *sender,
                     ammo::gateway::PullRequest &pr);
                     
protected:
  virtual bool matchedProjection (const Json::Value &root,
                                  const std::string &projection);
};

#endif /* EVENT_QUERY_HANDLER_H */