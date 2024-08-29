#pragma once

#include <string>
#include <pqxx/pqxx>
#include <memory>
#include <Json/json.h>
#include <future>
#include <unordered_set>
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

    std::shared_ptr<Json::Value> GetFreeTables(pqxx::result tables, pqxx::result reservations) {
        std::shared_ptr<Json::Value> jsonFreeTables = std::make_shared<Json::Value>();
        std::unordered_set<Id>isExistReservationSet;
        for (const auto& reservation : reservations) {
            Reservation res;
            res.SetPqxxRow(reservation);
            isExistReservationSet.insert(res.id);
        }
        for (const auto& table : tables) {
            Table tab;
            tab.SetPqxxRow(table);
            if (isExistReservationSet.find(tab.id) == isExistReservationSet.end())
                (*jsonFreeTables)["tables"].append(tab.numberOfSeats);
        }
        return jsonFreeTables;
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
    std::future<bool> getJsonArrayFreeReservations(std::shared_ptr<Json::Value> jsonFreeTables, std::string& DataBaseError, std::string date, std::string time) {
        return std::async(std::launch::async, [this, &jsonFreeTables, &DataBaseError, date, time]() {
             if (!isConnected()) {
                 DataBaseError = "Connection lost";
                 return false;
             }
             try {
                 pqxx::work txn(Connection);
                 pqxx::result reservations = txn.exec_params(
                     "SELECT id, time, date, table_id, user_id "
                     "FROM reservations "
                     "WHERE date = $1 AND time = $2;",
                     date, time
                 );

                 pqxx::result tables = txn.exec(
                     "SELECT id, number_of_seats "
                     "FROM tables;"
                 );

                 jsonFreeTables = GetFreeTables(reservations, tables);

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
                    "INSERT INTO reservations(time, date, table_id, user_id) "
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
    bool getIdFromLogin(const std::string login, Id& id, std::string DataBaseError) {
        if (!isConnected()) {
            DataBaseError = "Connection lost";
            return false;
        }
        try {
            pqxx::work txn(Connection);
            pqxx::result result = txn.exec_params(
                "SELECT id "
                "FROM users "
                "WHERE login = $1;",
                login
            );
            if (!result.empty()) {
                id = result[0]["id"].as<Id>();
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
    }
    bool getNumberOfSeatsFromId(const Id id, int& NumberOfSeats, std::string DataBaseError) {
        if (!isConnected()) {
            DataBaseError = "Connection lost";
            return false;
        }
        try {
            pqxx::work txn(Connection);
            pqxx::result result = txn.exec_params(
                "SELECT number_of_seats "
                "FROM tables "
                "WHERE id = $1;",
                id
            );

            NumberOfSeats = result[0]["number_of_seats"].as<int>();

            txn.commit();
            return true;
        }
        catch (const std::exception& e) {
            DataBaseError = e.what();
            return false;
        }
    }
    std::future<bool> getUserReservationJsonArray(const std::string login, std::shared_ptr<Json::Value>jsonArray, std::string DataBaseError) {
        return std::async(std::launch::async, [this, login, jsonArray, &DataBaseError]() {
            if (!isConnected()) {
                DataBaseError = "Connection lost";
                return false;
            }
            try {
                pqxx::work txn(Connection);

                Id loginId;
                bool getIdResult = getIdFromLogin(login, loginId, DataBaseError);
                if (!getIdResult) {
                    return false;
                }

                pqxx::result reservations = txn.exec_params(
                    "SELECT id, time, date table_id, user_id "
                    "FROM reservations "
                    "WHERE user_id = $1",
                    loginId
                );

                for (const auto& reservation : reservations) {
                    Reservation res;
                    res.SetPqxxRow(reservation);
                    std::shared_ptr<Json::Value> json = res.GetJson();

                    int NumberOfSeats;
                    bool getNumberResult = getNumberOfSeatsFromId(res.id, NumberOfSeats, DataBaseError);
                    if (!getNumberResult) {
                        return false;
                    }

                    (*json)["number_of_seats"] = NumberOfSeats;
                    (*jsonArray).append(*json);
                }

                return true;
            }
            catch (const std::exception& e) {
                DataBaseError = e.what();
                return false;
            }
        });
    }
    std::future<bool> deleteUserReservation(const Id id, std::string DataBaseError) {
        return std::async(std::launch::async, [this, id, &DataBaseError]() {
            if (!isConnected()) {
                DataBaseError = "Connection lost";
                return false;
            }
            try {
                pqxx::work txn(Connection);

                pqxx::result result = txn.exec_params(
                    "DELETE FROM reservations "
                    "WHERE id = $1;",
                    id
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
};
