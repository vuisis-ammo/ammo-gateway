#ifndef QUERY_STATEMENT_BUILDER_H
#define QUERY_STATEMENT_BUILDER_H

#include <string>

class sqlite3;
class sqlite3_stmt;

class QueryStatementBuilder
{
public:
  QueryStatementBuilder (const std::string &params,
                         sqlite3 *db,
                         const char *query_stub);

  ~QueryStatementBuilder (void);

  // Accesses the (finished) query statement.
  sqlite3_stmt *query (void) const;

protected:
  bool addFilter (const std::string &token,
                  const char *stub,
                  bool is_int);
                  
protected:
  const std::string &params_;
  sqlite3 *db_;
  sqlite3_stmt *stmt_;
  bool has_term_;
  std::string query_str_;
  std::string digits_;
};

#endif // QUERY_STATEMENT_BUILDER_H