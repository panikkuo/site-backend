#pragma once

#include <string>
#include <pqxx/pqxx>
#include <memory>
#include <Json/json.h>
#include <future>
#include <unordered_map>
#include <vector>

#include "Objects.h"

class DataBaseFunctions {
private:
    pqxx::connection Connection;

    std::string timeDurationToString(const boost::posix_time::time_duration& duration) {
        std::string hours = std::to_string(duration.hours());
        std::string minutes = std::to_string(duration.minutes());
        if (minutes.length() == 1) minutes = "0" + minutes;
        return hours + ":" + minutes;
    }
public:
    DataBaseFunctions(std::string dbname, std::string user, std::string password, std::string host, std::string port) :
        Connection("dbname=" + dbname +
            " user=" + user +
            " password=" + password +
            " host=" + host +
            " port=" + port)
    {}

    ~DataBaseFunctions() {
        Connection.close();
    }

    bool isConnected() {
        return Connection.is_open();
    }

    std::future<bool> getUserInfo(const std::string& login, User& user, std::string& DataBaseError) {
        return std::async(std::launch::async, [this, &login, &user, &DataBaseError]() {
            if (!isConnected()) {
                DataBaseError = "Connection lost";
                return false;
            }
            try {
                pqxx::work txn(Connection);
                pqxx::result result = txn.exec_params(
                    "SELECT id, login, password, name, email, phone "
                    "FROM users "
                    "WHERE login = $1;",
                    login
                );
                if (!result.empty()) {
                    user.SetPqxxRow(result[0]);
                    return true;
                }
                else {
                    DataBaseError = "User does not exist";
                    return false;
                }
            }
            catch (const std::exception& e) {
                DataBaseError = e.what();
                return false;
            }
        });
    }

    std::future<bool> addUser(const User& user, std::string& DataBaseError) {
        return std::async(std::launch::async, [this, user, &DataBaseError]() {
            if (!isConnected()) {
                DataBaseError = "Connection lost";
                return false;
            }
            try {
                pqxx::work txn(Connection);
                txn.exec_params(
                    "INSERT INTO users (login, password, name, email, phone) "
                    "VALUES ($1, $2, $3, $4, $5);",
                    user.login, user.password, user.name, user.email, user.phone
                );
                txn.commit();
                return true;
            }
            catch (const std::exception& e) {
                DataBaseError = e.what();
                return false;
            }
        });
    }

    std::future<bool> deleteUser(const std::string& login, std::string& DataBaseError) {
        return std::async(std::launch::async, [this, &login, &DataBaseError]() {
            if (!isConnected()) {
                DataBaseError = "Connection lost";
                return false;
            }
            try {
                pqxx::work txn(Connection);
                txn.exec_params(
                    "DELETE FROM users "
                    "WHERE login = $1; ",
                    login
                );
                txn.commit();
                return true;
            }
            catch (const std::exception& e) {
                DataBaseError = e.what();
                return false;
            }
        });
    }

    std::future<bool> doesValueExist(const std::string& column, const std::string& value, std::string& DataBaseError) {
        return std::async(std::launch::async, [this, &column, &value, &DataBaseError]() {
            if (!isConnected()) {
                DataBaseError = "Connection lost";
                return false;
            }
            try {
                pqxx::work txn(Connection);
                pqxx::result result = txn.exec_params(
                    "SELECT 1 "
                    "FROM users WHERE " + column + " = $1;",
                    value
                );
                return !result.empty();
            }
            catch (const std::exception& e) {
                DataBaseError = e.what();
                return false;
            }
        });
    }

    std::future<bool> getMenuJsonArray(std::shared_ptr<Json::Value> jsonArray, std::string& DataBaseError) {
        return std::async(std::launch::async, [this, jsonArray, &DataBaseError]() {
            if (!isConnected()) {
                DataBaseError = "Connection lost";
                return false;
            }
            try {
                pqxx::work txn(Connection);
                pqxx::result result = txn.exec(
                    "SELECT dish_id, name, description, price, image "
                    "FROM menu;"
                );

                for (const auto& row : result) {
                    Dish dish;
                    dish.SetPqxxRow(row);
                    std::shared_ptr<Json::Value> json = dish.GetJson();
                    (*jsonArray).append(*json);
                }

                txn.commit();
                return true;
            }
            catch (const std::exception& e) {
                DataBaseError = e.what();
                return false;
            }
        });
    }

    std::future<bool> getJsonArrayFreeReservations(std::shared_ptr<Json::Value> jsonArray, std::string& DataBaseError, std::string date) {
        return std::async(std::launch::async, [this, jsonArray, &DataBaseError, date]() {
            boost::gregorian::date requestDate (boost::gregorian::from_simple_string(date));
             if (!isConnected()) {
                 DataBaseError = "Connection lost";
                 return false;
             }
             try {
                 pqxx::work txn(Connection);
                 pqxx::result reservations = txn.exec_params(
                     "SELECT id, time, date, table_id, user_id "
                     "FROM reservations "
                     "WHERE date = $1;",
                     boost::gregorian::to_iso_extended_string(requestDate)
                 );

                 std::unordered_map<std::string, std::vector<Reservation>> timeToReservationsMap;
                 for (const auto& row : reservations) {
                     Reservation reservation;
                     reservation.SetPqxxRow(row);
                     timeToReservationsMap[timeDurationToString(reservation.time)].push_back(reservation);
                     //std::cout << timeDurationToString(reservation.time) << std::endl;
                 }

                 pqxx::result tables = txn.exec(
                     "SELECT id, number_of_seats "
                     "FROM tables;"
                 );

                 std::unordered_map<Id, Table> idToTableMap;

                 for (const auto& row : tables) {
                     Table table;
                     table.SetPqxxRow(row);
                     idToTableMap[table.id] = table;
                 }

                 Time startTime = boost::posix_time::hours(11);
                 Time endTime = boost::posix_time::hours(22);
                 Time step = boost::posix_time::minutes(30);
                 Time reservationGap = boost::posix_time::hours(2);

                 for (Time currentTime = startTime; currentTime <= endTime; currentTime += step) {
                     std::shared_ptr<Json::Value>json = std::make_shared<Json::Value>();
                     (*json)["time"] = timeDurationToString(currentTime);
                     for (const auto& reserv : timeToReservationsMap[timeDurationToString(currentTime)]) 
                         idToTableMap[reserv.tableId].reservedUntil = currentTime + reservationGap;

                     for (const auto& table : idToTableMap) 
                         if (table.second.reservedUntil <= currentTime) (*json)["number-of-seats"].append(table.second.numberOfSeats);          
                     
                     (*jsonArray).append(*json);
                 }
                 txn.commit();
                 return true;
             }
             catch (const std::exception& e) {
                 DataBaseError = e.what();
                 return false;
             }
        });
    }

    std::future<bool> addReservation(Reservation reservation, std::string& DataBaseError) {
        return std::async(std::launch::async, [this, reservation, &DataBaseError] {
            if (!isConnected()) {
                DataBaseError = "Connection lost";
                return false;
            }
            try {
                pqxx::work txn(Connection);
                pqxx::result result = txn.exec_params(
                    "INSERT VALUES(time, date, table_id, user_id) "
                    "VALUES($1, $2, $3, $4)",
                    timeDurationToString(reservation.time), boost::gregorian::to_iso_extended_string(reservation.date), reservation.tableId, reservation.userId
                );
                txn.commit();
                return true;
            }
            catch (const std::exception& e) {
                DataBaseError = e.what();
                return false;
            }
        });
    }
    std::future<bool> findTableWithSeatsNumber(const int NumberOfSeats, Table& table, std::string DataBaseError) {
        return std::async(std::launch::async, [this, NumberOfSeats, &table, &DataBaseError]() {
            if (!isConnected()) {
                DataBaseError = "Connection lost";
                return false;
            }
            try {
                pqxx::work txn(Connection);
                pqxx::result result = txn.exec_params(
                    "SELECT id, number_of_seats "
                    "FROM tables "
                    "WHERE number_of_seats = $1",
                    NumberOfSeats
                );

                if(!result.empty()) table.SetPqxxRow(result[0]);

                txn.commit();
                return true;
            }
            catch (const std::exception& e) {
                DataBaseError = e.what();
                return false;
            }
        });
    }
};
