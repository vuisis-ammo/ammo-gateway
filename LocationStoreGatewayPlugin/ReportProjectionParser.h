#ifndef REPORT_PROJECTION_PARSER_H
#define REPORT_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class ReportProjectionParser : public StringParser
{
public:
  ReportProjectionParser (void);

  void parse (const std::string &params);

  // Member accessors.
  const std::string &contentGuid (void) const;
  const std::string &reportTime_min (void) const;
  const std::string &reportTime_max (void) const;
  const std::string &reportingUnit (void) const;
  const std::string &size_min (void) const;
  const std::string &size_max (void) const;
  const std::string &activity (void) const;
  const std::string &locationUtm (void) const;
  const std::string &enemyUnit (void) const;
  const std::string &observationTime_min (void) const;
  const std::string &observationTime_max (void) const;
  const std::string &equipment (void) const;
  const std::string &assessment (void) const;
  const std::string &narrative (void) const;
  const std::string &authentication (void) const;

private:
  std::string contentGuid_;
  std::string reportTime_min_;
  std::string reportTime_max_;
  std::string reportingUnit_;
  std::string size_min_;
  std::string size_max_;
  std::string activity_;
  std::string locationUtm_;
  std::string enemyUnit_;
  std::string observationTime_min_;
  std::string observationTime_max_;
  std::string equipment_;
  std::string assessment_;
  std::string narrative_;
  std::string authentication_;
};

#endif // REPORT_PROJECTION_PARSER_H

