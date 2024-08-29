#pragma once

#include <drogon/drogon.h>
#include <memory>
#include <future>
#include <cstdio>

#include "JwtSystem.h"
#include "DataBaseMainFunctions.h"

class HandleRequestFunctions : public JwtFunctions {
private:
    void addCorsHeaders(drogon::HttpResponsePtr& response) {
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
    void OutputResponse(drogon::HttpResponsePtr response) {
        std::cout << "----------" << std::endl;
        for (const auto& header : response->getHeaders())
            std::cout << header.first << ": " << header.second << std::endl;
        std::cout << response->getBody() << std::endl;
        std::cout << "----------" << std::endl;
    }
    
public:
    void HandlePostRegisterUser(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::shared_ptr<DataBaseFunctions> DataBaseConnection) {
        drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
        std::shared_ptr<Json::Value> jsonResult = std::make_shared<Json::Value>();
        addCorsHeaders(response);

        if (request->method() == drogon::Post) {
            std::shared_ptr<Json::Value> jsonRequest = request->getJsonObject();
            if (!jsonRequest) {
                response->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
                (*jsonResult)["result"] = "Json is empty";
                response->setBody(jsonResult->toStyledString());
                callback(response);
                return;
            }
            User user;
            user.SetJson(jsonRequest);
            std::string DataBaseError;
            //TODO : Вот здесь бы переписать осозновая асинк Осознал но переписать сил нет
            auto checkLoginFuture = DataBaseConnection->doesValueExist("login", user.login, DataBaseError);
            auto checkEmailFuture = DataBaseConnection->doesValueExist("email", user.email, DataBaseError);
            auto checkPhoneFuture = DataBaseConnection->doesValueExist("phone", user.phone, DataBaseError);
            bool loginExists = checkLoginFuture.get();
            bool emailExists = checkEmailFuture.get();
            bool phoneExists = checkPhoneFuture.get();
            if (loginExists) {
                response->setStatusCode(drogon::HttpStatusCode::k409Conflict);
                (*jsonResult)["result"] = "Login already exists";
                response->setBody(jsonResult->toStyledString());
            }
            else if (emailExists) {
                response->setStatusCode(drogon::HttpStatusCode::k409Conflict);
                (*jsonResult)["result"] = "Email already exists";
                response->setBody(jsonResult->toStyledString());
            }
            else if (phoneExists) {
                response->setStatusCode(drogon::HttpStatusCode::k409Conflict);
                (*jsonResult)["result"] = "Phone number already exists";
                response->setBody(jsonResult->toStyledString());
            }
            else {
                std::string DataBaseError;
                auto addUserFuture = DataBaseConnection->addUser(user, DataBaseError);

                bool userAdded = addUserFuture.get();
                if (!userAdded) {
                    response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                    (*jsonResult)["result"] = DataBaseError;
                    callback(response);
                    return;
                }
                response->setStatusCode(drogon::HttpStatusCode::k200OK);
                (*jsonResult)["result"] = "User was added successfully";
                
            }
            OutputResponse(response);
            callback(response);
        }
        else if (request->getMethod() == drogon::Options) {
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            callback(response);
        }
        else {
            response->setStatusCode(drogon::HttpStatusCode::k405MethodNotAllowed);
            (*jsonResult)["result"] = "Method not allowed";
            response->setBody(jsonResult->toStyledString());
            callback(response);
        }
    }

    void HandlePostAuthorization(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::shared_ptr<DataBaseFunctions> DataBaseConnection) {
        drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
        std::shared_ptr<Json::Value>jsonResult = std::make_shared<Json::Value>();
        addCorsHeaders(response);

        if (request->method() == drogon::Post) {
            std::shared_ptr<Json::Value> jsonRequest = request->getJsonObject();

            if (!jsonRequest) {
                response->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
                (*jsonResult)["result"] = "Json is invalid";
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response); 
                return;
            }

            std::string checkLogin = (*jsonRequest)["login"].as<std::string>();
            std::string check_password = (*jsonRequest)["password"].as<std::string>();
            std::string DataBaseError;
            User user;

            auto getUserInfoFuture = DataBaseConnection->getUserInfo(checkLogin, user, DataBaseError);
            bool userInfoFetched = getUserInfoFuture.get();

            if (!userInfoFetched) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);     
                return;

            }
            if (user.isEmptyUser || user.password != check_password) {
                response->setStatusCode(drogon::HttpStatusCode::k409Conflict);
                (*jsonResult)["result"] = "Incorrect login or password";
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }
            std::string token = GetToken(checkLogin);
            response->addHeader("token", token);
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            (*jsonResult)["result"] = "Success";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);
            callback(response);
        }
        else if (request->getMethod() == drogon::Options) {
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            OutputResponse(response);
            callback(response);    
        }
        else {
            response->setStatusCode(drogon::HttpStatusCode::k405MethodNotAllowed);
            (*jsonResult)["result"] = "Method not allowed";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);
            callback(response);      
        }
    }

    void HandleGetDeleteLogin(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::shared_ptr<DataBaseFunctions> DataBaseConnection, const std::string& userLogin) {
        drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
        std::shared_ptr<Json::Value>jsonResult = std::make_shared<Json::Value>();
        addCorsHeaders(response);

        if (request->getMethod() == drogon::Get) {
            std::string DataBaseError;
            User user;

            auto getUserInfoFuture = DataBaseConnection->getUserInfo(userLogin, user, DataBaseError);
            bool userInfoFetched = getUserInfoFuture.get();

            if (!userInfoFetched) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return; 
            }

            if (user.isEmptyUser) {
                response->setStatusCode(drogon::HttpStatusCode::k404NotFound);
                (*jsonResult)["result"] = "User not found";
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }

            std::shared_ptr<Json::Value> json = user.GetJson();
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            response->setBody(json->toStyledString());
            OutputResponse(response);
            callback(response);
        }
        else if (request->getMethod() == drogon::Delete) {
            std::string DataBaseError;

            auto deleteUserFuture = DataBaseConnection->deleteUser(userLogin, DataBaseError);
            bool userDeleted = deleteUserFuture.get();

            if (!userDeleted) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }

            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            (*jsonResult)["result"] = "User was deleted successfully";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);
            callback(response);          
        }
        else if (request->getMethod() == drogon::Options) {
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            OutputResponse(response);
            callback(response);
        }
        else {
            response->setStatusCode(drogon::HttpStatusCode::k405MethodNotAllowed);
            (*jsonResult)["result"] = "Method not allowed";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);
            callback(response);  
        }    
    }

    void HandleGetMenu(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::shared_ptr<DataBaseFunctions> DataBaseConnection){
        drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
        std::shared_ptr<Json::Value>jsonResult = std::make_shared<Json::Value>();
        addCorsHeaders(response);

        if (request->getMethod() == drogon::Get) {
            std::shared_ptr<Json::Value> jsonArray = std::make_shared<Json::Value>(Json::arrayValue);
            std::string DataBaseError;

            auto getMenuFuture = DataBaseConnection->getMenuJsonArray(jsonArray, DataBaseError);
            bool menuFetched = getMenuFuture.get();

            if (!menuFetched) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
                
            }
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            response->setBody(jsonArray->toStyledString());
            OutputResponse(response);
            callback(response);
        }
        else {
            response->setStatusCode(drogon::HttpStatusCode::k405MethodNotAllowed);
            (*jsonResult)["result"] = "Method not allowed";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);
            callback(response);
        }
    }

    void HandleGetReservation(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::shared_ptr<DataBaseFunctions> DataBaseConnection, std::string date, std::string time) {
        drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
        std::shared_ptr<Json::Value>jsonResult = std::make_shared<Json::Value>();
        addCorsHeaders(response);

        if (request->getMethod() == drogon::Get) {
            std::shared_ptr<Json::Value> jsonArray = std::make_shared<Json::Value>(Json::arrayValue);

            std::string DataBaseError;

            auto getFreeReservationFuture = DataBaseConnection->getJsonArrayFreeReservations(jsonArray, DataBaseError, date, time);
            bool reservationFetched = getFreeReservationFuture.get();

            if (!reservationFetched) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);   
                return;
                
            }
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            response->setBody(jsonArray->toStyledString());
            OutputResponse(response);
            callback(response);

        }
        else {
            response->setStatusCode(drogon::HttpStatusCode::k405MethodNotAllowed);
            (*jsonResult)["result"] = "Method not allowed";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);
            callback(response);   
        }
    }

    void HandlePostReservation(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::shared_ptr<DataBaseFunctions> DataBaseConnection) {
        drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
        std::shared_ptr<Json::Value>jsonResult = std::make_shared<Json::Value>();
        addCorsHeaders(response);

        if (request->getMethod() == drogon::Post) {
            std::shared_ptr<Json::Value> jsonRequest = request->getJsonObject();
            std::string DataBaseError;

            if (!jsonRequest) {
                response->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
                (*jsonResult)["Result"] = "Json is invalid";
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }


            std::string login = (*jsonRequest)["login"].as<std::string>();
            std::string token = request->getHeader("token");
            if (token == "" || !CheckUserLoginToken(token, login)) {
                response->setStatusCode(drogon::HttpStatusCode::k403Forbidden);
                (*jsonResult)["result"] = "User is not authorized";
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }
            User user;

            auto getUserInfoFuture = DataBaseConnection->getUserInfo(login, user, DataBaseError);
            bool userFetched = getUserInfoFuture.get();

            if (!userFetched) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }

            int requestNumberOfSeats = (*jsonRequest)["number-of-seats"].as<int>();
            Table table;
            auto findTableFuture = DataBaseConnection->findTableWithSeatsNumber(requestNumberOfSeats, table, DataBaseError);
            bool tableFetched = findTableFuture.get();

            if (!tableFetched) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;

            }
            if (table.isEmptyTable) {
                response->setStatusCode(drogon::HttpStatusCode::k409Conflict);
                (*jsonResult)["result"] = "Table is not exist";
                response->setBody(jsonRequest->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }

            Reservation reservation;
            reservation.Set(0, (*jsonRequest)["date"].as<std::string>(), (*jsonRequest)["time"].as<std::string>(), table.id, user.id);
            auto addReservationFuture = DataBaseConnection->addReservation(reservation, DataBaseError);
            bool reservationFetched = addReservationFuture.get();

            if (!reservationFetched) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }

            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            (*jsonResult)["result"] = "Reservation was added successfully";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);   callback(response);
            
        }
        else if (request->getMethod() == drogon::Options) {
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            OutputResponse(response);
            callback(response);
        }
        else {
            response->setStatusCode(drogon::HttpStatusCode::k405MethodNotAllowed);
            (*jsonResult)["result"] = "Method not allowed";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);
            callback(response);
        }
    }
    void HandleGetUserReservation(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::shared_ptr<DataBaseFunctions> DataBaseConnection, const std::string& userLogin) {
        drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
        std::shared_ptr<Json::Value>jsonResult = std::make_shared<Json::Value>();
        addCorsHeaders(response);

        if (request->getMethod() == drogon::Get) {
            std::shared_ptr<Json::Value>jsonArray = std::make_shared<Json::Value>(Json::arrayValue);

            std::string DataBaseError;
            auto getUserReservationFututre = DataBaseConnection->getUserReservationJsonArray(userLogin, jsonArray, DataBaseError);
            bool getUserReservationFetched = getUserReservationFututre.get();

            if (!getUserReservationFetched) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }

            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            response->setBody(jsonArray->toStyledString());
            OutputResponse(response);
            callback(response);
        }
        else if (request->getMethod() == drogon::Options) {
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            OutputResponse(response);
            callback(response);
        }
        else {
            response->setStatusCode(drogon::HttpStatusCode::k405MethodNotAllowed);
            (*jsonResult)["result"] = "Method not allowed";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);
            callback(response);
        }
    }
    void HandleDeleteUserReservation(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::shared_ptr<DataBaseFunctions> DataBaseConnection, const std::string& userLogin, const Id id) {
        drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
        std::shared_ptr<Json::Value>jsonResult = std::make_shared<Json::Value>();
        addCorsHeaders(response);

        if (request->getMethod() == drogon::Delete) {
            std::string DataBaseError;
            auto getUserReservationFututre = DataBaseConnection->deleteUserReservation(id, DataBaseError);
            bool getUserReservationFetched = getUserReservationFututre.get();

            if (!getUserReservationFetched) {
                response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                (*jsonResult)["result"] = DataBaseError;
                response->setBody(jsonResult->toStyledString());
                OutputResponse(response);
                callback(response);
                return;
            }

            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            (*jsonResult)["result"] = "Reservation was deleted successfully";
            OutputResponse(response);
            callback(response);
        }
        else if (request->getMethod() == drogon::Options) {
            response->setStatusCode(drogon::HttpStatusCode::k200OK);
            OutputResponse(response);
            callback(response);
        }
        else {
            response->setStatusCode(drogon::HttpStatusCode::k405MethodNotAllowed);
            (*jsonResult)["result"] = "Method not allowed";
            response->setBody(jsonResult->toStyledString());
            OutputResponse(response);
            callback(response);
        }
    }
};
