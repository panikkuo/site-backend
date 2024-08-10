#include <iostream>

#include "Objects.h"
#include "DataBaseMainFunctions.h"
#include "HttpHanlderFunctions.h"
#include <drogon/HttpFilter.h>

std::shared_ptr<DataBaseFunctions> DataBaseConnection;
std::shared_ptr<HandleRequestFunctions> handleRequest;

//TODO : разобраться с картинками в диске(откуда диск) main task

void SetMethods() {
	drogon::app().registerHandler(
		"/api/v1/register",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
				handleRequest->HandlePostRegisterUser(request, std::move(callback), DataBaseConnection);
		},
		{ drogon::Post, drogon::Options }
	);
	drogon::app().registerHandler(
		"/api/v1/menu",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
				handleRequest->HandleGetMenu(request, std::move(callback), DataBaseConnection);
		},
		{ drogon::Get }
		);
	drogon::app().registerHandler(
		"/api/v1/authorization",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
				handleRequest->HandlePostAuthorization(request, std::move(callback), DataBaseConnection);
		},
		{ drogon::Post, drogon::Options }
	);
	drogon::app().registerHandler(
		"/api/v1/login/{1}",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string user_login) {
				handleRequest->HandleGetDeleteLogin(request, std::move(callback), DataBaseConnection, user_login);
		},
		{ drogon::Get, drogon::Delete, drogon::Options }
	);
	drogon::app().registerHandler(
		"/api/v1/reservation/{1}",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string date) {
				handleRequest->HandleGetReservation(request, std::move(callback), DataBaseConnection, date);
		},
		{ drogon::Get }
	);
	drogon::app().registerHandler(
		"/api/v1/reservation",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string date) {
				handleRequest->HandlePostReservation(request, std::move(callback), DataBaseConnection);
		},
		{ drogon::Post , drogon::Options}
		);
} 

int main() {
	DataBaseConnection = std::make_shared<DataBaseFunctions>("restaurant", "postgres", "nikanika228", "localhost", "5432");
	handleRequest = std::make_shared<HandleRequestFunctions>();

	SetMethods();

	drogon::app().addListener("26.33.89.53", 8080).run();

	return 1;
}