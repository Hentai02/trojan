/*
 * This file is part of the trojan project.
 * Trojan is an unidentifiable mechanism that helps you bypass GFW.
 * Copyright (C) 2017-2020  The Trojan Authors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "authenticator.h"
#include <cstdlib>
#include <stdexcept>
#include <sstream>
using namespace std;

#ifdef ENABLE_POSTGRESQL

Authenticator::Authenticator(const Config &config) {
    stringstream conn_str;
    conn_str << "host=" << config.postgresql.server_addr
             << " port=" << config.postgresql.server_port
             << " dbname=" << config.postgresql.database
             << " user=" << config.postgresql.username
             << " password=" << config.postgresql.password;
    
    if (!config.postgresql.ca.empty()) {
        conn_str << " sslmode=verify-ca sslrootcert=" << config.postgresql.ca;
        if (!config.postgresql.key.empty() && !config.postgresql.cert.empty()) {
            conn_str << " sslcert=" << config.postgresql.cert
                     << " sslkey=" << config.postgresql.key;
        }
    } else if (!config.postgresql.sslmode.empty()) {
        conn_str << " sslmode=" << config.postgresql.sslmode;
    }
    
    con = PQconnectdb(conn_str.str().c_str());
    
    if (PQstatus(con) != CONNECTION_OK) {
        string err = PQerrorMessage(con);
        PQfinish(con);
        throw runtime_error(err);
    }
    
    Log::log_with_date_time("connected to PostgreSQL server", Log::INFO);
}

bool Authenticator::auth(const string &password) {
    if (!is_valid_password(password)) {
        return false;
    }
    
    string query = "SELECT quota, download + upload FROM users WHERE password = $1";
    const char *param_values[1] = { password.c_str() };
    int param_lengths[1] = { static_cast<int>(password.length()) };
    int param_formats[1] = { 0 }; // text format
    
    PGresult *res = PQexecParams(con, query.c_str(), 1, nullptr, param_values, param_lengths, param_formats, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        Log::log_with_date_time(PQerrorMessage(con), Log::ERROR);
        PQclear(res);
        return false;
    }
    
    if (PQntuples(res) == 0) {
        PQclear(res);
        return false;
    }
    
    int64_t quota = atoll(PQgetvalue(res, 0, 0));
    int64_t used = atoll(PQgetvalue(res, 0, 1));
    PQclear(res);
    
    if (quota < 0) {
        return true;
    }
    if (used >= quota) {
        Log::log_with_date_time(password + " ran out of quota", Log::WARN);
        return false;
    }
    return true;
}

void Authenticator::record(const string &password, uint64_t download, uint64_t upload) {
    if (!is_valid_password(password)) {
        return;
    }
    
    stringstream query;
    query << "UPDATE users SET download = download + " << download
          << ", upload = upload + " << upload
          << " WHERE password = $1";
    
    const char *param_values[1] = { password.c_str() };
    int param_lengths[1] = { static_cast<int>(password.length()) };
    int param_formats[1] = { 0 };
    
    PGresult *res = PQexecParams(con, query.str().c_str(), 1, nullptr, param_values, param_lengths, param_formats, 0);
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        Log::log_with_date_time(PQerrorMessage(con), Log::ERROR);
    }
    PQclear(res);
}

bool Authenticator::is_valid_password(const string &password) {
    if (password.size() != PASSWORD_LENGTH) {
        return false;
    }
    for (size_t i = 0; i < PASSWORD_LENGTH; ++i) {
        if (!((password[i] >= '0' && password[i] <= '9') || (password[i] >= 'a' && password[i] <= 'f'))) {
            return false;
        }
    }
    return true;
}

Authenticator::~Authenticator() {
    if (con) {
        PQfinish(con);
    }
}

#else // ENABLE_POSTGRESQL

Authenticator::Authenticator(const Config&) : con(nullptr) {}
bool Authenticator::auth(const string&) { return true; }
void Authenticator::record(const string&, uint64_t, uint64_t) {}
bool Authenticator::is_valid_password(const string&) { return true; }
Authenticator::~Authenticator() {}

#endif // ENABLE_POSTGRESQL
