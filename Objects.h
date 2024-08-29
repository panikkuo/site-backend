#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <pqxx/pqxx>
#include <Json/json.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

typedef int Id;
typedef boost::gregorian::date Date;
typedef boost::posix_time::time_duration Time;

class User {
public: 
    Id id;
    std::string login;
    std::string password;
    std::string email;
    std::string name;
    std::string phone;
    bool isEmptyUser;
    User() {
        isEmptyUser = true;
        id = 0;
        login = "";
        password = "";
        email = "";
        name = "";
        phone = "";
    }
    void SetJson(const std::shared_ptr<Json::Value> json) {
        isEmptyUser = false;
        login = (*json)["login"].as<std::string>();
        password = (*json)["password"].as<std::string>();
        name = (*json)["name"].as<std::string>();
        email = (*json)["email"].as<std::string>();
        phone = (*json)["phone"].as<std::string>();
    }
    void SetPqxxRow(const pqxx::row row) {
        isEmptyUser = false;
        id = row["id"].as<int>();
        login = row["login"].as<std::string>();
        password = row["password"].as<std::string>();
        name = row["name"].as<std::string>();
        email = row["email"].as<std::string>();
        phone = row["phone"].as<std::string>();
    }
    std::shared_ptr<Json::Value> GetJson() {
        std::shared_ptr<Json::Value> json = std::make_shared<Json::Value>();

        (*json)["id"] = id;
        (*json)["login"] = login;
        (*json)["password"] = password;
        (*json)["name"] = name;
        (*json)["email"] = email;
        (*json)["phone"] = phone;

        return json;
    }
};

class Dish {
public:
    Id id;
    std::string name;
    std::string description;
    std::string price;
    std::string image;
    bool isEmptyDish;

    Dish() {
        isEmptyDish = true;
        id = 0;
        name = "";
        description = "";
        price = "";
        image = "";
    }
    void SetPqxxRow(const pqxx::row row) {
        isEmptyDish = false;
        id = row["dish_id"].as<int>();
        name = row["name"].as<std::string>();
        description = row["description"].as<std::string>();
        price = row["price"].as<std::string>();
        image = row["image"].as<std::string>();
    }
    void SetJson(std::shared_ptr<Json::Value> json) {
        isEmptyDish = false;
        name = (*json)["name"].as<int>();
        description = (*json)["description"].as<std::string>();
        price = (*json)["price"].as<std::string>();
        image = (*json)["image"].as<std::string>();
    }
    std::shared_ptr<Json::Value> GetJson() {
        std::shared_ptr<Json::Value> json = std::make_shared<Json::Value>();

        (*json)["id"] = id;
        (*json)["description"] = description;
        (*json)["name"] = name;
        (*json)["price"] = price;
        (*json)["image"] = image;

        return json;
    }
};

class Reservation {
private:
    std::string timeDurationToString(const boost::posix_time::time_duration& duration) {
        std::string hours = std::to_string(duration.hours());
        std::string minutes = std::to_string(duration.minutes());
        if (minutes.length() == 1) minutes = "0" + minutes;
        return hours + ":" + minutes;
    }
    std::string dateToString(const Date date) {
        std::string day = std::to_string(date.day());
        if (day.length() == 1) day = "0" + day;
        std::string month = std::to_string(date.month());
        if (month.length() == 1) month = "0" + month;
        return day + ":" + month + ":" + std::to_string(date.year());
    }
public:
    Id id;
    Date date;
    Time time;
    int tableId;
    int userId;
    bool isEmptyReservation;

    Reservation(){
        isEmptyReservation = true;
        id = 0;
        Date _date(2024, 1, 1);
        date = _date;
        Time _time(0, 0, 0);
        time = _time;
        tableId = 0;
        userId = 0;
    }
    void SetPqxxRow(const pqxx::row row) {
        isEmptyReservation = false;
        id = row["id"].as<int>();
        Date _date(boost::gregorian::from_simple_string(row["date"].as<std::string>()));
        date = _date;
        Time _time = boost::posix_time::duration_from_string(row["time"].as<std::string>());
        time = _time;
        tableId = row["table_id"].as<int>();
        userId = row["user_id"].as<int>();
    }
    void SetJson(const std::shared_ptr<Json::Value> json) {
        isEmptyReservation = false;
        Date _date(boost::gregorian::from_simple_string((*json)["date"].as<std::string>()));
        date = _date;
        Time _time = boost::posix_time::duration_from_string((*json)["time"].as<std::string>());
        time = _time;
        tableId = (*json)["table-id"].as<int>();
        userId = (*json)["user-id"].as<int>();
    }
    void Set(Id destinationId, std::string destinationDate, std::string destinationTime, Id destinationTableId, Id destinationUserId) {
        isEmptyReservation = true;
        id = destinationId;
        Date _date(boost::gregorian::from_simple_string(destinationDate));
        Time _time = boost::posix_time::duration_from_string(destinationTime);
        tableId = destinationTableId;
        userId = destinationUserId;
    }
    std::shared_ptr<Json::Value> GetJson() {
        std::shared_ptr<Json::Value> json = std::make_shared<Json::Value>();
        (*json)["id"] = id;
        (*json)["table-id"] = tableId;
        (*json)["user-id"] = userId;
        (*json)["date"] = dateToString(date);
        (*json)["time"] = timeDurationToString(time);
        return json;
    }
};

class Table {
public:
    Id id;
    int numberOfSeats;
    Time reservedUntil;
    bool isEmptyTable;
    Table() {
        isEmptyTable = true;
        id = 0;
        numberOfSeats = 0;
    }
    void SetPqxxRow(const pqxx::row row) {
        isEmptyTable = false;
        id = row["id"].as<int>();
        numberOfSeats = row["number_of_seats"].as<int>();
    }
    void SetJson(const std::shared_ptr<Json::Value> json) {
        id = (*json)["id"].as<int>();
        numberOfSeats = (*json)["number-of-seats"].as<int>();
    }
    void GetJson(std::shared_ptr<Json::Value> json) {
        (*json)["id"] = id;
        (*json)["number-of-seats"] = numberOfSeats;
    }
};
